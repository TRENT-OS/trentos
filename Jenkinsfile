pipeline {
    agent none
    stages {
        stage('checkout') {
            agent any
            steps {
                echo '##################################### Checkout COMPLETED ####################################'
            }
        }
        stage('build') {
            agent {
                docker {
                    image 'camkes_build_env_20190307'
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
        stage('prepare_test') {
            agent {
                docker {
                    image 'test_env'
                    args '-v /home/jenkins/.ssh/:/home/jenkins/.ssh:ro -v /etc/localtime:/etc/localtime:ro'
                }
            }
            options { skipDefaultCheckout(true) }
            steps {
                echo '####################################### Update TA ENV #######################################'
                sh  '''#!/bin/bash
                        if [ -d ta ]; then
                            cd ta
                            git pull origin
                            git submodule update --recursive
                        else
                            git clone --recursive -b master ssh://git@bitbucket.hensoldt-cyber.systems:7999/hc/ta.git
                            cd ta
                            python3 -m venv ta-env
                        fi
                        source ta-env/bin/activate
                        pip install -r requirements.txt
                    '''
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
                sh  '''#!/bin/bash
                        workspace=`pwd`
                        source ta/ta-env/bin/activate
                        cd ta/seos_tests
                        pytest -v
                    '''
            }
        }
    }
}
