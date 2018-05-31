package Cortex.buildTypes

import jetbrains.buildServer.configs.kotlin.v2017_2.*
import jetbrains.buildServer.configs.kotlin.v2017_2.buildSteps.script
import jetbrains.buildServer.configs.kotlin.v2017_2.triggers.VcsTrigger
import jetbrains.buildServer.configs.kotlin.v2017_2.triggers.finishBuildTrigger
import jetbrains.buildServer.configs.kotlin.v2017_2.triggers.vcs

object Cortex_WindowsCortexBuild : Template({
    uuid = "ad6aaf34-41fc-4280-8d6b-43c7c90c3cc3"
    id = "Cortex_WindowsCortexBuild"
    name = "Windows Cortex Build"

    artifactRules = "+:%system.teamcity.build.tempDir% => gafferDependenciesWithCortex.zip"
    buildNumberPattern = "%version%b%build.counter%"

    params {
        param("env.BOOST_MSVC_VERSION", "msvc-14.0")
        param("env.BUILD_DIR", "%system.teamcity.build.tempDir%")
        param("env.BUILD_TYPE", "RELEASE")
        param("env.CMAKE_GENERATOR", """"Visual Studio 15 2017 Win64"""")
        param("env.ROOT_DIR", "%system.teamcity.build.checkoutDir%")
        param("env.VERSION", "0.42.0.0")
    }

    vcs {
        root(Cortex.vcsRoots.Cortex_HttpsGithubComCortexRefsHeadsMaster)

    }

    steps {
        script {
            name = "Build Dependencies"
            id = "RUNNER_12"
            scriptContent = """
                call "%Windows Command Line Script Directory%\vcvars64.bat"
                call %teamcity.build.checkoutDir%\contrib\cmake\buildCortexWindows.bat
            """.trimIndent()
        }
        stepsOrder = arrayListOf("RUNNER_12")
    }

    triggers {
        vcs {
            id = "vcsTrigger"
            quietPeriodMode = VcsTrigger.QuietPeriodMode.USE_DEFAULT
            branchFilter = ""
            groupCheckinsByCommitter = true
        }
        finishBuildTrigger {
            id = "TRIGGER_1"
            buildTypeExtId = "GafferDependencies_GafferDependenciesForWindows"
            successfulOnly = true
            branchFilter = "+:%teamcity.build.branch%"
        }
    }

    dependencies {
        artifacts("GafferDependencies_GafferDependenciesForWindows") {
            id = "ARTIFACT_DEPENDENCY_1"
            buildRule = lastSuccessful("%teamcity.build.branch%")
            artifactRules = "+:gafferDependencies.zip!**=>%system.teamcity.build.tempDir%"
        }
    }

    requirements {
        equals("teamcity.agent.jvm.os.name", "Windows 10", "RQ_4")
    }
})
