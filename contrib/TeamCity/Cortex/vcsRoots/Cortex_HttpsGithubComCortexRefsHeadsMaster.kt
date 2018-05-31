package Cortex.vcsRoots

import jetbrains.buildServer.configs.kotlin.v2017_2.*
import jetbrains.buildServer.configs.kotlin.v2017_2.vcs.GitVcsRoot

object Cortex_HttpsGithubComCortexRefsHeadsMaster : GitVcsRoot({
    uuid = "384daee0-c605-43da-bdf4-c60abbab151d"
    id = "Cortex_HttpsGithubComCortexRefsHeadsMaster"
    name = "Cortex"
    url = "https://github.com/ImageEngine/cortex.git"
    branchSpec = "+:*"
    authMethod = password {
        userName = ""
        password = ""
    }
})
