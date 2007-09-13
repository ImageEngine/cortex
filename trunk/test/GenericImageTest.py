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

import unittest
import sys, math
from IECore import *

class TestImageReadersAndWriters(unittest.TestCase):
    """
    generic image reader and writer testing
    """

    def testHandlers(self):
        """
        test each reader and writer with grayscale (equal R,G,B channels)
        image
        """

        #print ''

        image_handlers = ["CIN", "JPEG", "TIFF", "EXR"]

        # construct the simple ImagePrimitive
        width = 16
        height = 16
        channel = {}
        channel["R"] = IECore.FloatVectorData(width * height)
        for i in range(0, width*height):
            channel["R"][i] = i / 255.0

        channel["G"] = IECore.FloatVectorData(width * height)
        for i in range(0, width*height):
            channel["G"][i] = ((i + 128)%(width*height-1)) / 255.0

        channel["B"] = IECore.FloatVectorData(width * height)
        for i in range(0, width*height):
            channel["B"][i] = ((i + 203)%(width*height-1)) / 255.0
		
        b = IECore.Box2i(IECore.V2i(0, 0), IECore.V2i(width-1, height-1))
        image = IECore.ImagePrimitive(b, b)
        image["R"] = IECore.PrimitiveVariable(IECore.PrimitiveVariable.Interpolation.Vertex, channel["R"])
        image["G"] = IECore.PrimitiveVariable(IECore.PrimitiveVariable.Interpolation.Vertex, channel["G"])
        image["B"] = IECore.PrimitiveVariable(IECore.PrimitiveVariable.Interpolation.Vertex, channel["B"])
        
        # test the handlers
        for image_type in image_handlers:
            
            # write the file
            writer = IECore.Writer.create(image, "test/generic_image.%s" % image_type.lower())
            self.assert_( writer.isInstanceOf( "ImageWriter" ) )
            self.assert_( writer["object"].isInstanceOf( "ObjectParameter" ) )
            self.assertEqual( writer["object"].validTypes(), [IECore.TypeId.ImagePrimitive] )
            self.assert_( writer.resultParameter().isInstanceOf( "ObjectParameter" ) )
            self.assertEqual( writer.resultParameter().validTypes(), [IECore.TypeId.ImagePrimitive] )
            writer.write()

            # read the file
            reader = IECore.Reader.create("test/generic_image.%s" % image_type.lower())
            self.assert_( reader.isInstanceOf( "ImageReader" ) )
            self.assert_( reader.resultParameter().isInstanceOf( "ObjectParameter" ) )
            self.assertEqual( reader.resultParameter().validTypes(), [IECore.TypeId.ImagePrimitive] )			
            read_image = reader.read()

            # write back out to verify
            IECore.Writer.create(read_image, "test/generic_image.blab.%s" % image_type.lower()).write()

            # compare pixel values
            for cn in ["R", "G", "B"]:
                read_channel = read_image[cn].data
                sum_of_diff = 0.0

                for i in range(0, width*height):

                    # big epsilon for now.  this should depend on the encoding.  for example,
                    # CIN will have some big errors because we quantize down to a 10-bit log space.
                    # JPEG hurts on these non-photolike images
                    epsilon = image_type == 'CIN' and 0.005 or image_type == 'JPEG' and 0.5 or 0.000001
                    diff = read_channel[i] - channel[cn][i]
                    sum_of_diff += diff
                    if math.fabs(diff) > epsilon:
                        print '%s channel %s: difference between write and read at %d: %f (%f read vs %f generated)' % (image_type, cn, i, diff, read_channel[i], channel[cn][i])
                        raise Exception("bad image write or read: encoding is %s" % image_type)

                if math.fabs(sum_of_diff) > (image_type == 'JPEG' and 0.2 or 0.1):
                    print 'sum of differences:', sum_of_diff
                    raise Exception("bad image write or reader: encoding is %s" % image_type)
            


    def testWindows(self):
        """
        test each reader and writer combo for windowed (axis-aligned sub-rectangle) operations
        """

        #print ''

        image_handlers = ["CIN", "JPEG", "TIFF", "EXR"]

        # construct the simple ImagePrimitive
        width = 16
        height = 16
        channel = IECore.FloatVectorData(width * height)
        for i in range(0, width*height):
            channel[i] = i / 255.0
		
        b = IECore.Box2i(IECore.V2i(0, 0), IECore.V2i(width-1, height-1))
        image = IECore.ImagePrimitive(b, b)
        image["R"] = IECore.PrimitiveVariable(IECore.PrimitiveVariable.Interpolation.Vertex, channel)
        image["G"] = IECore.PrimitiveVariable(IECore.PrimitiveVariable.Interpolation.Vertex, channel)
        image["B"] = IECore.PrimitiveVariable(IECore.PrimitiveVariable.Interpolation.Vertex, channel)

        # get smaller sub-window
        sub_box = Box2iData(Box2i(V2i(0, 0), V2i(7, 7)))
        
        # test the handlers
        for image_type in image_handlers:
            
            # write the file
            IECore.Writer.create(image, "test/generic_image.window.%s" % image_type.lower()).write()

            # read the file with a 1/4 area subwindow starting at origin
            r = IECore.Reader.create("test/generic_image.window.%s" % image_type.lower())
            r.parameters().dataWindow.setValue(sub_box)
            read_image = r.read()

            # write out the file
            IECore.Writer.create(read_image, "test/generic_image_cut.%s" % image_type.lower()).write()

            # assert proper image extents

            # compare pixel values
            for cn in ["R", "G", "B"]:
                read_channel = read_image[cn].data
                sum_of_diff = 0.0

                # big epsilon for now.  this should depend on the encoding.  for example,
                # CIN will have some big errors because we quantize down to a 10-bit log space.
                epsilon = image_type == 'CIN' and 0.005 or image_type == 'JPEG' and 0.00025 or 0.000001

                for y in range(0, 8):
                    for x in range(0, 8):

                        ci = 16*y + x
                        i = 8*y + x
                        
                        diff = read_channel[i] - channel[ci]

                        sum_of_diff += diff
                        if math.fabs(diff) > epsilon:
                            print '%s channel %s: difference between write and read at %d: %f' % (image_type, cn, i, diff)
                            raise Exception("bad image write or read: encoding is %s" % image_type)


                if math.fabs(sum_of_diff) > 0.1:
                    print 'sum of differences:', sum_of_diff
                    raise Exception("bad image write or reader: encoding is %s" % image_type)
            



    def testOutsideWindows(self):
        """
        test each reader and writer combo for windowed (axis-aligned sub-rectangle) operations
        """

        #print ''

        image_handlers = ["CIN", "JPEG", "TIFF", "EXR"]

        # construct the simple ImagePrimitive
        width = 16
        height = 16
        channel = IECore.FloatVectorData(width * height)
        for i in range(0, width*height):
            channel[i] = i / 255.0
		
        b = IECore.Box2i(IECore.V2i(0, 0), IECore.V2i(width-1, height-1))
        image = IECore.ImagePrimitive(b, b)
        image["R"] = IECore.PrimitiveVariable(IECore.PrimitiveVariable.Interpolation.Vertex, channel)
        image["G"] = IECore.PrimitiveVariable(IECore.PrimitiveVariable.Interpolation.Vertex, channel)
        image["B"] = IECore.PrimitiveVariable(IECore.PrimitiveVariable.Interpolation.Vertex, channel)

        # get smaller sub-window
        sub_box = Box2iData(Box2i(V2i(-4, -4), V2i(21, 21)))
        
        # test the handlers
        for image_type in image_handlers:
            
            # write the file
            IECore.Writer.create(image, "test/generic_image.outside.%s" % image_type.lower()).write()

            # read the file with a 1/4 area subwindow starting at origin
            r = IECore.Reader.create("test/generic_image.outside.%s" % image_type.lower())
            r.parameters().dataWindow.setValue(sub_box)
            read_image = r.read()

            # write out the file
            IECore.Writer.create(read_image, "test/generic_image_outside.%s" % image_type.lower()).write()

            # assert proper image extents
            # ...

            # compare pixel values
            for cn in ["R", "G", "B"]:
                read_channel = read_image[cn].data
                sum_of_diff = 0.0

                # big epsilon for now.  this should depend on the encoding.  for example,
                # CIN will have some big errors because we quantize down to a 10-bit log space.
                epsilon = image_type == 'CIN' and 0.005 or image_type == 'JPEG' and 0.00025 or 0.000001

                for y in range(0, 26):
                    for x in range(0, 26):

                        i = 26*y + x
                        
                        if y < 4 or x < 4 or y >= 4 + 16 or x >= 4 + 16:
                            if read_channel[i] > 0.0001:
                                raise Exception("non-zero channel value (%f) in outside region (x,y %d,%d) of windowed read (%s encoding)" % (read_channel[i], x, y, image_type))

                        else:
                            
                            ci = 16*(y-4) + (x-4)
                        
                            diff = read_channel[i] - channel[ci]

                            sum_of_diff += diff
                            if math.fabs(diff) > epsilon:
                                print '%s channel %s: difference between write and read at %d, %d: %f' % (image_type, cn, x, y, diff)
                                raise Exception("bad image write or read: encoding is %s" % image_type)
                            
                            
                if math.fabs(sum_of_diff) > 0.1:
                    print 'sum of differences:', sum_of_diff
                    raise Exception("bad image write or reader: encoding is %s" % image_type)
            

            
        
        
                                        
if __name__ == "__main__":
        unittest.main()   
        
