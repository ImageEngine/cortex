import unittest
import sys
import IECore

class TestOBJReader(unittest.TestCase):

    def testRead(self):

        self.testfile = 'test/data/obj/triangle.obj'
        
        r = IECore.Reader.create(self.testfile)
        self.assertEqual(type(r), IECore.OBJReader)
        
        mesh = r.read()

    def testReadNormals(self):

        self.testfile = 'test/data/obj/triangle_normals.obj'
        
        r = IECore.Reader.create(self.testfile)
        self.assertEqual(type(r), IECore.OBJReader)
        
        mesh = r.read()

                                        

    def testReadNoTexture(self):

        self.testfile = 'test/data/obj/triangle_no_texture.obj'
        
        r = IECore.Reader.create(self.testfile)
        self.assertEqual(type(r), IECore.OBJReader)
        
        mesh = r.read()


    def testGroups(self):

        self.testfile = 'test/data/obj/groups.obj'
        
        r = IECore.Reader.create(self.testfile)
        self.assertEqual(type(r), IECore.OBJReader)
        
        mesh = r.read()
                                        

                                        
if __name__ == "__main__":
        unittest.main()   
