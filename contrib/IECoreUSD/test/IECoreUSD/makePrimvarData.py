import os
from pxr import Usd, UsdGeom, Tf, Sdf, Vt, Gf

# Helper script to generate a USD file with a single sphere primitive with all the primvar types on it
# both array and scalar
# just for reference in case we need to generate ./data/primVars.usda again

usdTypes = {
	"Bool" : [Vt.Bool( False ), Vt.BoolArray( [False, True] )],
	"UChar" : [Vt.UChar( 0 ), Vt.UCharArray( [0, 1, 2] )],
	"Int" : [Vt.Int( -1 ), Vt.IntArray( [0, -1, -2] )],
	"UInt" : [Vt.UInt( 4294967295 ), Vt.UIntArray( [4294967293, 4294967294, 4294967295] )],
	"UInt64" : [Vt.UInt64( 18446744073709551615 ), Vt.UInt64Array( [18446744073709551613, 18446744073709551614, 18446744073709551615] )],
	"Int64" : [Vt.Int64( -9223372036854775808 ), Vt.Int64Array( [9223372036854775805, 9223372036854775806, 9223372036854775807] )],
	"Half" : [Vt.Half( 0.5 ), Vt.HalfArray( [0.1, 0.2, 0.3] )],
	"Float" : [Vt.Float( 0.6 ), Vt.FloatArray( [0.7, 0.8, 0.9] )],
	"Double" : [Vt.Double( 1.1 ), Vt.DoubleArray( [1.2, 1.3, 1.4] )],
	"String" : ["this", ["is", "a", "test"]],
	"Token" : ["t-this", ["t-is", "t-a", "t-test"]],
	"Int2" : [Gf.Vec2i( 1, 2 ), [Gf.Vec2i( 3, 4 ), Gf.Vec2i( 5, 6 ), Gf.Vec2i( 7, 8 )]],
	"Int3" : [Gf.Vec3i( 1, 2, 3 ), [Gf.Vec3i( 3, 4, 5 ), Gf.Vec3i( 5, 6, 7 ), Gf.Vec3i( 7, 8, 9 )]],
	"Int4" : [Gf.Vec4i( 1, 2, 3, 4 ), [Gf.Vec4i( 3, 4, 5, 6 ), Gf.Vec4i( 5, 6, 7, 8 ), Gf.Vec4i( 7, 8, 9, 10 )]],
	"Half2" : [Gf.Vec2h( 0.1, 0.2 ), [Gf.Vec2h( 1.1, 1.2 ), Gf.Vec2h( 2.1, 2.2 ), Gf.Vec2h( 3.1, 3.2 )]],
	"Half3" : [Gf.Vec3h( 0.1, 0.2, 0.3 ), [Gf.Vec3h( 1.1, 1.2, 1.3 ), Gf.Vec3h( 2.1, 2.2, 2.3 ), Gf.Vec3h( 3.1, 3.2, 3.3 )]],
	"Half4" : [Gf.Vec4h( 0.1, 0.2, 0.3, 0.4 ), [Gf.Vec4h( 1.1, 1.2, 1.3, 1.4 ), Gf.Vec4h( 2.1, 2.2, 2.3, 2.4 ), Gf.Vec4h( 3.1, 3.2, 3.3, 3.4 )]],
	"Float2" : [Gf.Vec2f( 0.1, 0.2 ), [Gf.Vec2f( 1.1, 1.2 ), Gf.Vec2f( 2.1, 2.2 ), Gf.Vec2f( 3.1, 3.2 )]],
	"Float3" : [Gf.Vec3f( 0.1, 0.2, 0.3 ), [Gf.Vec3f( 1.1, 1.2, 1.3 ), Gf.Vec3f( 2.1, 2.2, 2.3 ), Gf.Vec3f( 3.1, 3.2, 3.3 )]],
	"Float4" : [Gf.Vec4f( 0.1, 0.2, 0.3, 0.4 ), [Gf.Vec4f( 1.1, 1.2, 1.3, 1.4 ), Gf.Vec4f( 2.1, 2.2, 2.3, 2.4 ), Gf.Vec4f( 3.1, 3.2, 3.3, 3.4 )]],
	"Double2" : [Gf.Vec2d( 0.1, 0.2 ), [Gf.Vec2d( 1.1, 1.2 ), Gf.Vec2d( 2.1, 2.2 ), Gf.Vec2d( 3.1, 3.2 )]],
	"Double3" : [Gf.Vec3d( 0.1, 0.2, 0.3 ), [Gf.Vec3d( 1.1, 1.2, 1.3 ), Gf.Vec3d( 2.1, 2.2, 2.3 ), Gf.Vec3d( 3.1, 3.2, 3.3 )]],
	"Double4" : [Gf.Vec4d( 0.1, 0.2, 0.3, 0.4 ), [Gf.Vec4d( 1.1, 1.2, 1.3, 1.4 ), Gf.Vec4d( 2.1, 2.2, 2.3, 2.4 ), Gf.Vec4d( 3.1, 3.2, 3.3, 3.4 )]],
	"point3h" : [Gf.Vec3h( 0.1, 0.2, 0.3 ), [Gf.Vec3h( 1.1, 1.2, 1.3 ), Gf.Vec3h( 2.1, 2.2, 2.3 ), Gf.Vec3h( 3.1, 3.2, 3.3 )]],
	"point3f" : [Gf.Vec3f( 0.1, 0.2, 0.3 ), [Gf.Vec3f( 1.1, 1.2, 1.3 ), Gf.Vec3f( 2.1, 2.2, 2.3 ), Gf.Vec3f( 3.1, 3.2, 3.3 )]],
	"point3d" : [Gf.Vec3d( 0.1, 0.2, 0.3 ), [Gf.Vec3d( 1.1, 1.2, 1.3 ), Gf.Vec3d( 2.1, 2.2, 2.3 ), Gf.Vec3d( 3.1, 3.2, 3.3 )]],
	"vector3h" : [Gf.Vec3h( 0.1, 0.2, 0.3 ), [Gf.Vec3h( 1.1, 1.2, 1.3 ), Gf.Vec3h( 2.1, 2.2, 2.3 ), Gf.Vec3h( 3.1, 3.2, 3.3 )]],
	"vector3f" : [Gf.Vec3f( 0.1, 0.2, 0.3 ), [Gf.Vec3f( 1.1, 1.2, 1.3 ), Gf.Vec3f( 2.1, 2.2, 2.3 ), Gf.Vec3f( 3.1, 3.2, 3.3 )]],
	"vector3d" : [Gf.Vec3d( 0.1, 0.2, 0.3 ), [Gf.Vec3d( 1.1, 1.2, 1.3 ), Gf.Vec3d( 2.1, 2.2, 2.3 ), Gf.Vec3d( 3.1, 3.2, 3.3 )]],
	"normal3h" : [Gf.Vec3h( 0.1, 0.2, 0.3 ), [Gf.Vec3h( 1.1, 1.2, 1.3 ), Gf.Vec3h( 2.1, 2.2, 2.3 ), Gf.Vec3h( 3.1, 3.2, 3.3 )]],
	"normal3f" : [Gf.Vec3f( 0.1, 0.2, 0.3 ), [Gf.Vec3f( 1.1, 1.2, 1.3 ), Gf.Vec3f( 2.1, 2.2, 2.3 ), Gf.Vec3f( 3.1, 3.2, 3.3 )]],
	"normal3d" : [Gf.Vec3d( 0.1, 0.2, 0.3 ), [Gf.Vec3d( 1.1, 1.2, 1.3 ), Gf.Vec3d( 2.1, 2.2, 2.3 ), Gf.Vec3d( 3.1, 3.2, 3.3 )]],
	"color3h" : [Gf.Vec3h( 0.1, 0.2, 0.3 ), [Gf.Vec3h( 1.1, 1.2, 1.3 ), Gf.Vec3h( 2.1, 2.2, 2.3 ), Gf.Vec3h( 3.1, 3.2, 3.3 )]],
	"color3f" : [Gf.Vec3f( 0.1, 0.2, 0.3 ), [Gf.Vec3f( 1.1, 1.2, 1.3 ), Gf.Vec3f( 2.1, 2.2, 2.3 ), Gf.Vec3f( 3.1, 3.2, 3.3 )]],
	"color3d" : [Gf.Vec3d( 0.1, 0.2, 0.3 ), [Gf.Vec3d( 1.1, 1.2, 1.3 ), Gf.Vec3d( 2.1, 2.2, 2.3 ), Gf.Vec3d( 3.1, 3.2, 3.3 )]],
	"color4h" : [Gf.Vec4h( 0.1, 0.2, 0.3, 0.4 ), [Gf.Vec4h( 1.1, 1.2, 1.3, 1.4 ), Gf.Vec4h( 2.1, 2.2, 2.3, 2.4 ), Gf.Vec4h( 3.1, 3.2, 3.3, 3.4 )]],
	"color4f" : [Gf.Vec4f( 0.1, 0.2, 0.3, 0.4 ), [Gf.Vec4f( 1.1, 1.2, 1.3, 1.4 ), Gf.Vec4f( 2.1, 2.2, 2.3, 2.4 ), Gf.Vec4f( 3.1, 3.2, 3.3, 3.4 )]],
	"color4d" : [Gf.Vec4d( 0.1, 0.2, 0.3, 0.4 ), [Gf.Vec4d( 1.1, 1.2, 1.3, 1.4 ), Gf.Vec4d( 2.1, 2.2, 2.3, 2.4 ), Gf.Vec4d( 3.1, 3.2, 3.3, 3.4 )]],
	"quath" : [Gf.Quath( 0, 0, 0, 1 ), [Gf.Quath( 1, 0, 0, 0 ), Gf.Quath( 0, 1, 0, 0 ), Gf.Quath( 0, 0, 1, 0 )]],
	"quatf" : [Gf.Quatf( 0, 0, 0, 1 ), [Gf.Quatf( 1, 0, 0, 0 ), Gf.Quatf( 0, 1, 0, 0 ), Gf.Quatf( 0, 0, 1, 0 )]],
	"quatd" : [Gf.Quatd( 0, 0, 0, 1 ), [Gf.Quatd( 1, 0, 0, 0 ), Gf.Quatd( 0, 1, 0, 0 ), Gf.Quatd( 0, 0, 1, 0 )]],
	"matrix2d" : [Gf.Matrix2d( 1, 0, 0, 0 ), [Gf.Matrix2d( 0, 1, 0, 0 ), Gf.Matrix2d( 0, 0, 2, 0 ), Gf.Matrix2d( 0, 0, 3, 0 )]],
	"matrix3d" : [Gf.Matrix3d( 0, 0, 0, 0, 0, 0, 0, 0, 0 ), [Gf.Matrix3d( 0, 0, 0, 0, 1, 0, 0, 0, 0 ), Gf.Matrix3d( 0, 0, 0, 0, 0, 0, 0, 0, 2 ), Gf.Matrix3d( 0, 0, 4, 0, 0, 0, 0, 0, 0 )]],
	"matrix4d" : [Gf.Matrix4d( 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ),
		[Gf.Matrix4d( 1, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 0, 0, 0, 0, 0 ), Gf.Matrix4d( 1, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ), Gf.Matrix4d( 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0 )]],
}

interpolations = ['constant']
arrayVariations = [False, True]


fileName = os.path.join( os.path.dirname( __file__ ), "data/primVars.usda")

stage = Usd.Stage.CreateNew( fileName )

xformPrim = UsdGeom.Xform.Define( stage, '/root' )
spherePrim = UsdGeom.Points.Define( stage, '/root/sphere' )

for usdType, testData in usdTypes.items() :
	scalarTestData, arrayTestData = testData
	for isArray in arrayVariations :
		for interpolation in interpolations :

			fullType = usdType
			if isArray :
				fullType += "[]"

			fullType = fullType.lower()

			primVar = spherePrim.CreatePrimvar( attrName = "test_{0}_{1}_{2}".format( usdType, "Array" if isArray else "Scalar", interpolation ), typeName = Sdf.ValueTypeNames.Find( fullType ),
				interpolation = interpolation )
			attr = primVar.GetAttr()
			data = arrayTestData if isArray else scalarTestData
			attr.Set( data )

stage.GetRootLayer().Save()
