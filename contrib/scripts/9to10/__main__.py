import os
import sys
from lib2to3 import main

sys.path.append( os.path.dirname( os.path.dirname( __file__ ) ) )
sys.exit( main.main( "9to10" ) )
