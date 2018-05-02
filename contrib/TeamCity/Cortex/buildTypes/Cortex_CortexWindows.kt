package Cortex.buildTypes

import jetbrains.buildServer.configs.kotlin.v2017_2.*

object Cortex_CortexWindows : BuildType({
    template(Cortex.buildTypes.Cortex_WindowsCortexBuild)
    uuid = "51aff0af-a6f0-4bd3-b6c2-0cf402738ba5"
    id = "Cortex_CortexWindows"
    name = "Cortex Windows"
})
