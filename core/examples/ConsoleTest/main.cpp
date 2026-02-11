/*-*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-   */
/*ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab:*/

/*
  Author(s): Peter Kazanzides, Haochen Wei

  (C) Copyright 2024 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#include <string>

#include <cisstCommon/cmnLogger.h>
#include <cisstCommon/cmnKbHit.h>
#include <cisstCommon/cmnGetChar.h>
#include <cisstCommon/cmnConstants.h>
#include <cisstOSAbstraction/osaSleep.h>
#include <cisstMultiTask/mtsManagerLocal.h>
#include <cisstMultiTask/mtsTaskContinuous.h>
#include <cisstMultiTask/mtsInterfaceRequired.h>

#include <sawMaxonEPOS/mtsMaxonEPOS.h>

class MaxonClient : public mtsTaskMain {
private:
    std::ifstream fin;
    std::istream* in = &std::cin; 

    size_t NumAxes;
    vctDoubleVec jtpgoal, jtvgoal;
    vctDoubleVec jtpos, jtvel;

    prmStateJoint m_measured_js;
    prmStateJoint m_setpoint_js;
    prmPositionJointSet jtposSet;
    prmVelocityJointSet jtvelSet;
    prmOperatingState m_op_state;
    prmActuatorState m_ActuatorState;

    mtsFunctionRead measured_js;
    mtsFunctionRead setpoint_js;
    mtsFunctionRead operating_state;
    mtsFunctionRead GetActuatorState;
    mtsFunctionWrite servo_jp;
    mtsFunctionWrite move_jp;
    mtsFunctionWrite servo_jv;
    mtsFunctionWrite state_command;

    mtsFunctionRead period_statistics;
    mtsIntervalStatistics period_stats;

    void OnStatusEvent(const mtsMessage &msg) {
        std::cout << std::endl << "Status: " << msg.Message << std::endl;
    }
    void OnWarningEvent(const mtsMessage &msg) {
        std::cout << std::endl << "Warning: " << msg.Message << std::endl;
    }
    void OnErrorEvent(const mtsMessage &msg) {
        std::cout << std::endl << "Error: " << msg.Message << std::endl;
    }

public:

    MaxonClient() : mtsTaskMain("MaxonClient"), NumAxes(0)
    {
        mtsInterfaceRequired *req = AddInterfaceRequired("Input");
        if (req) {
            req->AddFunction("measured_js", measured_js);
            req->AddFunction("setpoint_js", setpoint_js);
            req->AddFunction("operating_state", operating_state);
            req->AddFunction("GetActuatorState", GetActuatorState);
            req->AddFunction("servo_jp", servo_jp);
            req->AddFunction("move_jp", move_jp);
            req->AddFunction("servo_jv", servo_jv);
            req->AddFunction("state_command", state_command);
            req->AddFunction("period_statistics", period_statistics);
        }
    }

    void Configure(const std::string& fileName) {
        fin.open(fileName);
        if (!fin) {
            std::cerr << "Failed to open file\n";
            return;
        }
        in = &fin;
    }

    void PrintHelp()
    {
        std::cout << "Available commands:" << std::endl
                  << "  m: position move joints (servo_jp)" << std::endl
                  << "  p: profile move joints (move_jr)" << std::endl
                  << "  v: velocity move joints (servo_jv)" << std::endl
                  << "  c: script move" << std::endl
                  << "  s: stop move (hold)" << std::endl
                  << "  h: display help information" << std::endl
                  << "  e: enable motor power" << std::endl
                  << "  n: disable motor power" << std::endl
                  << "  a: get actuator state" << std::endl
                  << "  o: get operating state" << std::endl
                  << "  q: quit" << std::endl;
    }

    void Startup()
    {
        NumAxes = 0;
        const mtsGenericObject *p = measured_js.GetArgumentPrototype();
        const prmStateJoint *psj = dynamic_cast<const prmStateJoint *>(p);
        if (psj) NumAxes = psj->Position().size();
        std::cout << "MaxonClient: Detected " << NumAxes << " axes" << std::endl;
        jtpos.SetSize(NumAxes);
        jtvel.SetSize(NumAxes);

        jtpgoal.SetSize(NumAxes);
        jtvgoal.SetSize(NumAxes);
        
        jtposSet.Goal().SetSize(NumAxes);
        jtvelSet.SetSize(NumAxes);
        PrintHelp();
    }

    void Run() {

        ProcessQueuedEvents();
        measured_js(m_measured_js);
        setpoint_js(m_setpoint_js);
        m_measured_js.GetPosition(jtpos);
        m_measured_js.GetVelocity(jtvel);
        operating_state(m_op_state);
        period_statistics(period_stats);

        char c = 0;
        size_t i;
        size_t pulse;
        if (cmnKbHit()) {
            c = cmnGetChar();
            switch (c) {

            case 'm':   // position move joint
                std::cout << std::endl << "Enter joint positions (encoder count): ";
                for (i = 0; i < NumAxes; i++)
                    std::cin >> jtpgoal[i];
                std::cout << "Moving to " << jtpgoal << std::endl;
                jtposSet.SetGoal(jtpgoal);
                servo_jp(jtposSet);
                break;

            case 'p':   // profile move joint
                std::cout << std::endl << "Enter joint positions (encoder count): ";
                for (i = 0; i < NumAxes; i++)
                    std::cin >> jtpgoal[i];
                std::cout << "Relative move by " << jtpgoal << std::endl;
                jtposSet.SetGoal(jtpgoal);
                move_jp(jtposSet);
                break;

            case 'v':   // velocity move joint
                std::cout << std::endl << "Enter joint velocities (encoder count/s): ";
                for (i = 0; i < NumAxes; i++)
                    std::cin >> jtvgoal[i];
                jtvelSet.SetGoal(jtvgoal);
                servo_jv(jtvelSet);
                break;
            
            case 'c':   // velocity move joint
                std::cout << std::endl << "Move with input script";
                pulse = 0;
                while (true){
                    pulse++;
                    measured_js(m_measured_js);
                    m_measured_js.GetPosition(jtpos);
                    m_measured_js.GetVelocity(jtvel);
                    printf("POS: [");
                    for (i = 0; i < jtpos.size(); i++)
                        printf(" %7.2lf ", jtpos[i]);
                    printf("] VELOCITY: [");
                    for (i = 0; i < jtvel.size(); i++)
                        printf(" %7.2lf ", jtvel[i]);
                    printf("]\r");
                    osaSleep(0.01);
                    
                    bool ok=true;
                    
                    if (pulse < 99){
                        continue;
                    }    
                    pulse = 0;

                    for (i = 0; i < NumAxes; i++){
                        
                        if (!((*in) >> jtpgoal[i])) {
                            ok = false;
                            break;
                        }
                    }

                    if (!ok) break;

                    std::cout << "Moving to " << jtpgoal << std::endl;
                    jtposSet.SetGoal(jtpgoal);
                    servo_jp(jtposSet);
                    
                }
                std::cout << "Script finished" << std::endl;
                break;
            
            case 's':   // stop move (hold)
                state_command(std::string("pause"));
                std::cout << " System pause" << std::endl;
                break;
                
            case 'e':   // enable motor power
                state_command(std::string("enable"));
                std::cout << " System enable" << std::endl;
                break;

            case 'n':   // disable motor power
                state_command(std::string("disable"));
                std::cout << " System disabled" << std::endl;
                break;

            case 'h':
                std::cout << std::endl;
                PrintHelp();
                break;

            case 'a':
                GetActuatorState(m_ActuatorState);
                std::cout << std::endl << "ActuatorState: " << std::endl << m_ActuatorState << std::endl;
                break;

            case 'o':
                std::cout << std::endl << "Operating state: " << m_op_state << std::endl;
                break;

            case 'q':   // quit program
                std::cout << std::endl << "Exiting.. " << std::endl;
                state_command(std::string("disable"));      // Disable motor power
                this->Kill();
                break;

            }
        }

        printf("POS: [");
        for (i = 0; i < jtpos.size(); i++)
            printf(" %7.2lf ", jtpos[i]);
        printf("] VELOCITY: [");
        for (i = 0; i < jtvel.size(); i++)
            printf(" %7.2lf ", jtvel[i]);
        printf("]\r");

        // std::cout<<"XXX: "<<period_stats.PeriodAvg()<<std::endl;

        osaSleep(0.01);  // to avoid taking too much CPU time
    }

    void Cleanup() {}

};

int main(int argc, char **argv)
{
    cmnLogger::SetMask(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskDefaultLog(CMN_LOG_ALLOW_ALL);
    cmnLogger::SetMaskFunction(CMN_LOG_ALLOW_ALL);

    if (argc < 2) {
        std::cout << "Syntax: sawMaxonConsole <config>" << std::endl;
        std::cout << "        <config>      Configuration file (JSON format)" << std::endl;
        std::cout << "        <script>      Moving trace file (txt format)" << std::endl;
        return 0;
    }

    std::cout << "Starting mtsMaxonEPOS" << std::endl;
    mtsMaxonEPOS *MaxonServer;
    MaxonServer = new mtsMaxonEPOS("MaxonServer");
    MaxonServer->Configure(argv[1]);

    mtsComponentManager * componentManager = mtsComponentManager::GetInstance();
    componentManager->AddComponent(MaxonServer);

    MaxonClient client;
    if (argc == 3) {
        client.Configure(argv[2]);
    }
    componentManager->AddComponent(&client);

    if (!componentManager->Connect(client.GetName(), "Input", MaxonServer->GetName(), "I2RIS")) {
        std::cout << "Failed to connect: "
            << client.GetName() << "::Input to "
            << MaxonServer->GetName() << "::control" << std::endl;
        delete MaxonServer;
        return -1;
    }
    componentManager->CreateAll();
    componentManager->StartAll();

    // Main thread passed to client task

    componentManager->KillAll();
    componentManager->WaitForStateAll(mtsComponentState::FINISHED, 2.0 * cmn_s);
    componentManager->Cleanup();

    // stop all logs
    cmnLogger::SetMask(CMN_LOG_ALLOW_NONE);
    delete MaxonServer;

    return 0;
}
