
// bind the localtime to docker container to avoid problems of gaps between the
// localtime of the container and the host.
// add to group "stack" in order to grant usage of Haskell stack in the docker
// image

def DOCKER_BUILD_ENV = [ image: 'seos_build_env_20191010',
                         args: ' -v /etc/localtime:/etc/localtime:ro '+
                               ' --group-add=1001'
                       ]

def DOCKER_TEST_ENV  = [ image: 'seos_test_env_20191010',
                         args: ' -v /home/jenkins/.ssh/:/home/jenkins/.ssh:ro'+
                               ' -v /etc/localtime:/etc/localtime:ro' +
                               ' --network=host'+
                               ' --cap-add=NET_ADMIN' +
                               ' --device=/dev/net/tun'
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
        skipDefaultCheckout true
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
        stage('build_doc') {
            agent {
                docker {
                    reuseNode true
                    image DOCKER_BUILD_ENV.image
                    args DOCKER_BUILD_ENV.args
                }
            }
            options { skipDefaultCheckout(true) }
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
            options { skipDefaultCheckout(true) }
            steps {
                print_step_info env.STAGE_NAME
                sh 'scm-src/build.sh all-projects'
            }
        }
        stage('prepare_test') {
            agent {
                docker {
                    reuseNode true
                    image DOCKER_TEST_ENV.image
                    args DOCKER_TEST_ENV.args
                }
            }
            options { skipDefaultCheckout(true) }
            steps {
                print_step_info env.STAGE_NAME
                sh 'scm-src/test.sh prepare'
            }
        }
        stage('test') {
            agent {
                docker {
                    reuseNode true
                    image DOCKER_TEST_ENV.image
                    args DOCKER_TEST_ENV.args
                }
            }
            options { skipDefaultCheckout(true) }
            steps {
                print_step_info env.STAGE_NAME
                sh 'scm-src/test.sh run --junitxml=$WORKSPACE/test_results.xml --ignore-glob=test_network*'
            }
        }
        stage('test_network') {
            agent {
                docker {
                    reuseNode true
                    image DOCKER_TEST_ENV.image
                    args DOCKER_TEST_ENV.args
                }
            }
            options { skipDefaultCheckout(true) }
            steps {
                lock('nw_test_lock'){
                    print_step_info env.STAGE_NAME
                    sh 'scm-src/test.sh run --junitxml=$WORKSPACE/test_network_results.xml test_network*'
                }
            }
        }

        stage('astyle_check') {
            // run this after the tests, so we have test results even if source formatting is still not fine.
            options { skipDefaultCheckout(true) }
            steps {
                print_step_info env.STAGE_NAME
                sh 'scm-src/build.sh check_astyle_artifacts'
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
