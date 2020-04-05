
// bind the localtime to docker container to avoid problems of gaps between the
// localtime of the container and the host.
// add to group "stack" in order to grant usage of Haskell stack in the docker
// image

def DOCKER_BUILD_ENV = [
    image: 'seos_build_env_20191010',
    args:  ['-v /etc/localtime:/etc/localtime:ro',
            '--group-add=1001'
           ].join(' ')
]

def DOCKER_TEST_ENV = [
    registry: 'https://docker:5000',
    image:    'docker:5000/seos_test_env:test',
    args:     ['-v /home/jenkins/.ssh/:/home/jenkins/.ssh:ro',
               '-v /etc/localtime:/etc/localtime:ro',
               '--network=bridge',
               '--cap-add=NET_ADMIN',
               '--cap-add=NET_RAW',
               '--device=/dev/net/tun',
               '--group-add=sudo'
              ].join(' ')
]


def agentLabel = ( env.BRANCH_NAME in ["master", "integration"] ) ?
                 "jenkins_primary_slave"
                 : "jenkins_secondary_slave"

def print_step_info(name) { echo "#################### " + name }

pipeline {
    agent {
        label agentLabel
    }
    options {
        skipDefaultCheckout()
        disableConcurrentBuilds()

        // unfortunately there is no way to have build discarding conditional
        // for development branches only, so "master" and "integration" would
        // keep their builds. The potential work around is having a conditional
        // stage where "master" and "integration" push a package to an artifact
        // server, which then preserves this.
        buildDiscarder(logRotator(numToKeepStr: '20'))
    }
    environment {
        TEST_RUN_BASE = "test-logs-${env.BUILD_TIMESTAMP}"
    }
    stages {
        stage('workspace_cleanup') {
            when {
                expression { return (env.BRANCH_NAME in ["master", "integration"])  }
            }
            steps {
                print_step_info env.STAGE_NAME
                cleanWs()
            }
        }
        stage('checkout') {
            steps {
                print_step_info env.STAGE_NAME
                // everything is in separate folders to avoid file conflicts. Sources are checked out into
                // "scm-src", builds should generate "build-xxx" folders, tests will use "workspace_test" ...
                dir('scm-src') {
                    checkout scm
                }
            }
        }
        stage('build_doc') {
            agent {
                docker {
                    reuseNode true
                    image DOCKER_BUILD_ENV.image
                    args DOCKER_BUILD_ENV.args
                }
            }
            steps {
                print_step_info env.STAGE_NAME
                sh 'scm-src/build.sh doc'
            }
        }
        stage('build') {
            agent {
                docker {
                    reuseNode true
                    image DOCKER_BUILD_ENV.image
                    args DOCKER_BUILD_ENV.args
                }
            }
            steps {
                print_step_info env.STAGE_NAME
                sh 'scm-src/build.sh all-projects'
            }
        }
        stage('astyle_check') {
            // there is no need to run this in a container
            steps {
                print_step_info env.STAGE_NAME
                catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
                    sh 'scm-src/build.sh check_astyle_artifacts'
                }
            }
        }
        stage('prepare_test') {
            agent {
                docker {
                    reuseNode true
                    alwaysPull true
                    registryUrl DOCKER_TEST_ENV.registry
                    image DOCKER_TEST_ENV.image
                    args DOCKER_TEST_ENV.args
                }
            }
            steps {
                print_step_info env.STAGE_NAME
                sh 'scm-src/test.sh prepare'
            }
        }
        stage('test') {
            agent {
                docker {
                    reuseNode true
                    alwaysPull true
                    registryUrl DOCKER_TEST_ENV.registry
                    image DOCKER_TEST_ENV.image
                    args DOCKER_TEST_ENV.args
                }
            }
            environment {
                TEST_RUN_ID = "${env.TEST_RUN_BASE}-${env.STAGE_NAME}"
            }
            steps {
                print_step_info env.STAGE_NAME
                catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
                    sh 'scm-src/test.sh                      \
                            run                              \
                            test_demo_hello_world.py         \
                            test_chanmux.py                  \
                            test_proxy_nvm.py                \
                            test_crypto_api.py               \
                            test_partition_manager.py        \
                            test_filesystem_as_lib.py        \
                            test_config_server.py            \
                            test_config_server_fs_backend.py \
                            test_keystore.py                 \
                            test_logserver.py                \
                            test_cryptoserver.py             \
                            test_demo_iot_app.py'
                }
            }
        }
        stage('test_network') {
            agent {
                docker {
                    reuseNode true
                    alwaysPull true
                    registryUrl DOCKER_TEST_ENV.registry
                    image DOCKER_TEST_ENV.image
                    args DOCKER_TEST_ENV.args
                }
            }
            environment {
                TEST_RUN_ID = "${env.TEST_RUN_BASE}-${env.STAGE_NAME}"
            }
            steps {
                print_step_info env.STAGE_NAME
                catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
                    lock('nw_test_lock'){
                        sh 'scm-src/test.sh          \
                                run                  \
                                test_network_api.py  \
                                test_tls_api.py'
                    }
                }
            }
        }
    }
    post {
        always {
            print_step_info 'process test results'
            // running junit in a stage will prefix all the test with the stage
            // name. This is not useful for us, because we just need the two
            // different stages ro run network related tests separately. Thus
            // we run jeunit here for all results of this pipeline run.
            catchError {
                junit env.TEST_RUN_BASE + '*/test_results.xml'
            }

            print_step_info 'archive artifacts'
            // An archive with binaries (ie the build-* folders) is 530 MiB,
            // this makes CI run out of disk space quickly. Thus we just
            // archive the logs, which take about 300 KiB only. We have
            // dedicated build/test runs for each system also, they create an
            // archive with binaries and logs.
            sh 'tar -cjf build.bz2 \
                    ' + env.TEST_RUN_BASE + '*/ \
                    build-DOC/SEOS-Projects_doc-html'
            archiveArtifacts artifacts: 'build.bz2', fingerprint: true
        }
    }
}
