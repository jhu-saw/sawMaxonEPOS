/*-*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-   */
/*ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab:*/

/*
  Author(s): Peter Kazanzides

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
    size_t NumAxes;
    vctDoubleVec jtgoal, jtvel;
    vctDoubleVec jtpos;

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
        mtsInterfaceRequired *req = AddInterfaceRequired("Input", MTS_OPTIONAL);
        if (req) {
            req->AddFunction("measured_js", measured_js);
            req->AddFunction("setpoint_js", setpoint_js);
            req->AddFunction("operating_state", operating_state);
            req->AddFunction("GetActuatorState", GetActuatorState);
            req->AddFunction("servo_jp", servo_jp);
            req->AddFunction("move_jp", move_jp);
            req->AddFunction("servo_jv", servo_jv);
            req->AddFunction("state_command", state_command);
        }
    }

    void Configure(const std::string&) {}

    void PrintHelp()
    {
        std::cout << "Available commands:" << std::endl
                  << "  m: position move joints (servo_jp)" << std::endl
                  << "  r: relative move joints (servo_jr)" << std::endl
                  << "  v: velocity move joints (servo_jv)" << std::endl
                  << "  s: stop move (hold)" << std::endl
                  << "  c: send command" << std::endl
                  << "  h: display help information" << std::endl
                  << "  e: enable motor power" << std::endl
                  << "  n: disable motor power" << std::endl
                  << "  a: get actuator state" << std::endl
                  << "  o: get operating state" << std::endl
                  << "  i: display header info" << std::endl
                  << "  z: home robot" << std::endl
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
        jtgoal.SetSize(NumAxes);
        jtvel.SetSize(NumAxes);
        jtposSet.Goal().SetSize(NumAxes);
        jtvelSet.SetSize(NumAxes);
        PrintHelp();
    }

    void Run() {

        ProcessQueuedEvents();
        measured_js(m_measured_js);
        setpoint_js(m_setpoint_js);
        m_measured_js.GetPosition(jtpos);
        operating_state(m_op_state);

        char c = 0;
        size_t i;
        if (cmnKbHit()) {
            c = cmnGetChar();
            switch (c) {

            case 'm':   // position move joint
                std::cout << std::endl << "Enter joint positions (mm): ";
                for (i = 0; i < NumAxes; i++)
                    std::cin >> jtgoal[i];
                std::cout << "Moving to " << jtgoal << std::endl;
                jtposSet.SetGoal(jtgoal);
                servo_jp(jtposSet);
                break;

            case 'p':   // profile move joint
                std::cout << std::endl << "Enter relative joint positions (mm): ";
                for (i = 0; i < NumAxes; i++)
                    std::cin >> jtgoal[i];
                std::cout << "Relative move by " << jtgoal << std::endl;
                jtposSet.SetGoal(jtgoal);
                move_jp(jtposSet);
                break;

            case 'v':   // velocity move joint
                std::cout << std::endl << "Enter joint velocities (mm/s): ";
                for (i = 0; i < NumAxes; i++)
                    std::cin >> jtvel[i];
                jtvelSet.SetGoal(jtvel);
                servo_jv(jtvelSet);
                break;

            case 's':   // stop move (hold)
                state_command(std::string("pause"));
                break;
                
            case 'e':   // enable motor power
                state_command(std::string("enable"));
                break;

            case 'n':   // disable motor power
                state_command(std::string("disable"));
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
        // printf("] TORQUE: [");
        // vctDoubleVec jtt;
        // m_setpoint_js.GetEffort(jtt);
        // for (i = 0; i < jtt.size(); i++)
        //     printf(" %7.2lf ", jtt[i]);
        printf("]\r");

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
        return 0;
    }

    std::cout << "Starting mtsMaxonEPOS" << std::endl;
    mtsMaxonEPOS *MaxonServer;
    MaxonServer = new mtsMaxonEPOS("MaxonServer");
    MaxonServer->Configure(argv[1]);

    mtsComponentManager * componentManager = mtsComponentManager::GetInstance();
    componentManager->AddComponent(MaxonServer);

    MaxonClient client;
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

    MaxonServer->Kill();
    componentManager->WaitForStateAll(mtsComponentState::FINISHED, 2.0 * cmn_s);

    componentManager->Cleanup();

    // stop all logs
    cmnLogger::SetMask(CMN_LOG_ALLOW_NONE);
    delete MaxonServer;

    return 0;
}
