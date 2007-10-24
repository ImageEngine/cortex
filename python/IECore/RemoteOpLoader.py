##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#     * Neither the name of Image Engine Design nor the names of any
#       other contributors to this software may be used to endorse or
#       promote products derived from this software without specific prior
#       written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

import os, sys, imp, glob
import urllib
from xml.dom import minidom

from fnmatch import fnmatch
from IECore import ClassLoader, SearchPath, ParameterParser, Msg, msg, Op

class RemoteOpLoader(ClassLoader):
	"""
	Remote Op Loader
	"""

	def __init__(self, remoteHost, certificate):

		# set our remote host
		self.__remoteHost = remoteHost
		self.__certificate = minidom.parseString(certificate).documentElement

		# construct search path
		pdoc = self.fetch_document('<get-resolved-op-paths/>')
		pde = pdoc.documentElement
		env = str(':'.join(map(lambda v: v.getAttribute('value'), [cn for cn in pde.childNodes if cn.nodeName == 'path'])))
		sp = SearchPath(env, ':')

		# call the base-class init
		ClassLoader.__init__(self, sp)

	@classmethod
	def defaultRemoteOpLoader(cls, remoteHost, certificate):
		"""
		uncached class instance factory
		"""

		# fetch the OP_PATHS from the remote host.  this is published via the
		# <get-op-paths/> call.
		# here is a whole bunch of protocol knowledge that would not be required
		# if we were to fetch the op interface (that is, the parameters) from the
		# server.

		return cls(remoteHost, certificate)

	def load(self, name, version = None):

		# get the version here
		if not version:
			version = self.getDefaultVersion(name)

		# lookup in cache
		c = self._ClassLoader__classes[name]
		if version in c["imports"]:
			return c["imports"][version]

		# call the superclass method to load the class
		result = ClassLoader.load(self, name, version)
		if not issubclass(result, Op):
			raise Exception('RemoteOpLoader may only load() Op-based classes')

		# now mutate the class
		old_operate = result.operate
		def wrapped_operate(original):
			def wrapped(obj):

				# serialize parameters
				pp = ParameterParser()
				
				p = pp.serialise(obj.parameters())

				doc = minidom.DOMImplementation().createDocument('', 'request', '')
				de = doc.documentElement

				# add the certificate
				de.appendChild(doc.importNode(self.__certificate, True))

				# build the request
				req = doc.createElement('operate')
				req.setAttribute('name', name)
				req.setAttribute('version', str(version))
				req.setAttribute('parameters', p)
				de.appendChild(req)

				# build request string
				reqstr = doc.toprettyxml('  ')

				# contact server
				post = urllib.urlopen(self.__remoteHost, reqstr)
				postreply = post.read()
				post.close()

				# take the appropriate action on the reply.  notably:
				# construct exceptions if they were thrown
				# construct the return code for the Op
				#print 'post reply:'
				#print postreply

				retdoc = minidom.parseString(postreply)
				rde = retdoc.documentElement

				# handle the return value
				if rde.nodeName == 'exception':
					message = rde.childNodes[0].nodeValue
					e = eval('%s(message)' % rde.getAttribute('type'))
					raise e

				# parse out the return value
				result = pp.parse(str(rde.getAttribute('result')), obj.resultParameter())
				return obj.resultParameter()
				
				
			return wrapped

		result.operate = wrapped_operate(old_operate)
		result.remote = True

		# (re)cache and return: we should have a lock on the ClassLoader dictionary
		c = self._ClassLoader__classes[name]
		c["imports"][version] = result
		return result


	# fetch a document from the remoteHost server
	def fetch_document(self, request):

		doc = minidom.DOMImplementation().createDocument('', 'request', '')
		de = doc.documentElement
		de.appendChild(doc.importNode(self.__certificate, True))
		de.appendChild(minidom.parseString(request).documentElement)

		# now make the request
		req = doc.toprettyxml('  ')
		post = urllib.urlopen(self.__remoteHost, req)
		postreply = post.read()
		post.close()
		
		return minidom.parseString(postreply)


	def refresh(self):
	
		# remember any old defaultVersions
		defaultVersions = {}
		for k, v in self._ClassLoader__classes.items():
			defaultVersions[k] = v.get("defaultVersion", None)
	
		self._ClassLoader__classes = {}

		#
		# grab the classes from the server
		#
		cdoc = self.fetch_document('<list-ops/>')
		cde = cdoc.documentElement
		for cn in cde.childNodes:
			if cn.nodeName == 'op':

				# op := @name, @path, version+
				# version := @id

				classname = str(os.path.join(cn.getAttribute('path'), cn.getAttribute('name')))
				#print 'classname is:', classname

				for vn in cn.childNodes:
					if vn.nodeName == 'version':
						version = int(vn.getAttribute('id'))
						c = self._ClassLoader__classes.setdefault(classname, {'versions':[], 'imports':{}})
						c['versions'].append(version)

		# sort versions
		for c in self._ClassLoader__classes.values():
			c["versions"].sort()
				
		# restore old default versions
		for k, v in defaultVersions.items():
			if k in self._ClassLoader__classes and v:
				self.setDefaultVersion(k, v)
