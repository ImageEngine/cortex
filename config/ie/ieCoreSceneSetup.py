import os

import IECoreScene

IECoreScene.SharedSceneInterfaces.setMaxScenes( int(os.environ.get( "IECORESCENE_SHAREDSCENES_SIZE", 200 )) )
