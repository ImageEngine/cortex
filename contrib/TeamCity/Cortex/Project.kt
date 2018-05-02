package Cortex

import Cortex.buildTypes.*
import Cortex.vcsRoots.*
import jetbrains.buildServer.configs.kotlin.v2017_2.*
import jetbrains.buildServer.configs.kotlin.v2017_2.Project

object Project : Project({
    uuid = "c2f61ede-29db-4a08-ad61-d0a6fa082059"
    id = "Cortex"
    parentId = "Gaffer"
    name = "Cortex"

    vcsRoot(Cortex_HttpsGithubComRefsHeadsMaster)

    buildType(Cortex_CortexWindows)

    template(Cortex_WindowsCortexBuild)
})
