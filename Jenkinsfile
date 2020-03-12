
// bind the localtime to docker container to avoid problems of gaps between the
// localtime of the container and the host.
// add to group "stack" in order to grant usage of Haskell stack in the docker
// image

def DOCKER_BUILD_ENV = [ image: 'seos_build_env_20191010',
                         args: ' -v /etc/localtime:/etc/localtime:ro '+
                               ' --group-add=1001'
                       ]

def DOCKER_TEST_ENV = [
    image:      'docker:5000/seos_test_env:latest',
    args:       ' -v /home/jenkins/.ssh/:/home/jenkins/.ssh:ro'+
                    ' -v /etc/localtime:/etc/localtime:ro' +
                    ' --network=host' +
                    ' --cap-add=NET_ADMIN' +
                    ' --cap-add=NET_RAW' +
                    ' --device=/dev/net/tun',
    registry:   'http://docker:5000'
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
        stage('docker_update') {
            steps {
                print_step_info env.STAGE_NAME
                sh 'docker pull ' + DOCKER_TEST_ENV.image
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
                    registryUrl DOCKER_TEST_ENV.registry
                    image DOCKER_TEST_ENV.image
                    args DOCKER_TEST_ENV.args
                }
            }
            steps {
                print_step_info env.STAGE_NAME
                catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
                    sh '''scm-src/test.sh run                       \
                            --junitxml=$WORKSPACE/test_results.xml  \
                            test_hello_world.py                     \
                            test_chanmux.py                         \
                            test_proxy_nvm.py                       \
                            test_crypto_api.py                      \
                            test_partition_manager.py               \
                            test_filesystem_as_lib.py               \
                            test_config_server.py                   \
                            test_config_server_fs_backend.py        \
                            test_keystore.py                        \
                            test_logserver.py                       \
                            test_cryptoserver.py'''
                }
            }
        }
        stage('test_network') {
            agent {
                docker {
                    reuseNode true
                    registryUrl DOCKER_TEST_ENV.registry
                    image DOCKER_TEST_ENV.image
                    args DOCKER_TEST_ENV.args
                }
            }
            steps {
                lock('nw_test_lock'){
                    print_step_info env.STAGE_NAME
                    catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
                        sh '''scm-src/test.sh run                              \
                                --junitxml=$WORKSPACE/test_network_results.xml \
                                test_network_api.py                            \
                                test_tls_api.py'''
                    }
                }
            }
        }
    }
    post {
        always {
            junit '**/test_results.xml'
            junit '**/test_network_results.xml'
        }
    }
}
