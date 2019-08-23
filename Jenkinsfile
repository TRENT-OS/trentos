pipeline {
    agent none
    options {
        skipDefaultCheckout true
    }
    stages {
        stage('workspace_cleanup') {
            agent any
            when {
                   expression { return (env.BRANCH_NAME == 'integration' || env.BRANCH_NAME == 'master')  }
            }
            steps {
                echo '##################################### Workspace Cleanup ####################################'
                cleanWs()
            }
        }
        stage('checkout') {
            agent any
            steps {
                echo '######################################### Checkout #########################################'
                checkout scm
            }
        }
        stage('build_doc') {
            agent {
                docker {
                    image 'camkes_build_env_20190709'
                    // bind the localtime to avoid problems of gaps between the localtime of the container and the host
                    args '-v /etc/localtime:/etc/localtime:ro'
                }
            }
            options { skipDefaultCheckout(true) }
            steps {
                echo '############################## Building SeOS Documentation ##################################'
                sh './build.sh doxygen'
            }
        }
        stage('build') {
            agent {
                docker {
                    image 'camkes_build_env_20190709'
                    // bind the localtime to avoid problems of gaps between the localtime of the container and the host
                    args '-v /etc/localtime:/etc/localtime:ro'
                }
            }
            options { skipDefaultCheckout(true) }
            steps {
                echo '########################################## Building #########################################'
                // trigger the build
                sh './build.sh all'
            }
        }
        stage('astyle_check') {
            agent any
            options { skipDefaultCheckout(true) }
            steps {
                echo '####################################### Astyle Check ########################################'
                sh  '''#!/bin/bash -ue
                    files=`find . -name '*.astyle'`
                    if [ ! -z "$files" ]; then
                        exit 1
                    fi
                 '''
            }
        }
        stage('prepare_test') {
            agent {
                docker {
                    image 'test_env'
                    args '-v /home/jenkins/.ssh/:/home/jenkins/.ssh:ro -v /etc/localtime:/etc/localtime:ro'
                }
            }
            options { skipDefaultCheckout(true) }
            steps {
                echo '####################################### Prepare Test Environment ############################'
                sh './test.sh prepare'
            }
        }
        stage('test') {
            agent {
                 docker {
                     image 'test_env'
                     args '-v /home/jenkins/.ssh/:/home/jenkins/.ssh:ro -v /etc/localtime:/etc/localtime:ro'
                 }
            }
            options { skipDefaultCheckout(true) }
            steps {
                echo '########################################## Testing ##########################################'
                sh './test.sh run'
            }
        }
    }
}
