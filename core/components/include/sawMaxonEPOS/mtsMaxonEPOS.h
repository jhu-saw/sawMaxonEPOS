/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-    */
/* ex: set filetype=cpp softtabstop=4 shiftwidth=4 tabstop=4 cindent expandtab: */

/*
  Author(s): Haochen Wei, Peter Kazanzides, Anton Deguet

  (C) Copyright 2025 Johns Hopkins University (JHU), All Rights Reserved.

--- begin cisst license - do not edit ---

This software is provided "as is" under an open source license, with
no warranty.  The complete license can be found in license.txt and
http://www.cisst.org/cisst/license.txt.

--- end cisst license ---
*/

#ifndef _mtsGalilEPOS_h
#define _mtsGalilEPOS_h

#include <string>
#include <cstdint>

#include <cisstCommon/cmnPath.h>
#include <cisstVector/vctDynamicVectorTypes.h>
#include <cisstMultiTask/mtsTaskContinuous.h>
#include <cisstMultiTask/mtsInterfaceProvided.h>
#include <cisstParameterTypes/prmConfigurationJoint.h>
#include <cisstParameterTypes/prmStateJoint.h>
#include <cisstParameterTypes/prmPositionJointSet.h>
#include <cisstParameterTypes/prmVelocityJointSet.h>
#include <cisstParameterTypes/prmPositionCartesianGet.h>
#include <cisstParameterTypes/prmOperatingState.h>
#include <cisstParameterTypes/prmActuatorState.h>

// Always include last
#include <sawMaxonEPOS/sawMaxonEPOSExport.h>

class CISST_EXPORT mtsMaxonEPOS : public mtsTaskContinuous
{
    CMN_DECLARE_SERVICES(CMN_DYNAMIC_CREATION_ONEARG, CMN_LOG_LOD_RUN_ERROR)

 public:

    mtsMaxonEPOS(const std::string &name);
    mtsMaxonEPOS(const std::string &name, unsigned int sizeStateTable, bool newThread = true);
    mtsMaxonEPOS(const mtsTaskContinuousConstructorArg &arg);

    ~mtsMaxonEPOS();

    // cisstMultiTask functions
    void Configure(const std::string &fileName) override;
    void Startup(void) override;
    void Run(void) override;
    void Cleanup(void) override;

protected:

    // Path to configuration files
    cmnPath mConfigPath;

    // Structure for robot data
    struct RobotData {
        std::string   name;                     // Robot name (from config file)
        std::string   deviceName;
        std::string   protocolStackName;
        std::string   interfaceName;
        std::string   portName;
        
        unsigned int  baudrate;
        unsigned int  mTimeout;                 // Timeout

        unsigned int  mNumAxes;                 // Number of axes

        prmStateJoint m_measured_js;            // Measured joint state (CRTK)
        prmStateJoint m_setpoint_js;            // Setpoint joint state (CRTK)
        vctDoubleVec offset_js;                 // read offset for zero.
        
        prmOperatingState m_op_state;           // Operating state (CRTK)
        prmOperatingState::StateType newState;
        mtsFunctionWrite operating_state;       // Event generator

        prmActuatorState mActuatorState;        // Actuator state

        vctUIntVec    mAxisToNodeIDMap;         // Map from axis number to nodeID

        vctUIntVec    mState;                   // Internal axis state machine
        
        mtsInterfaceProvided *mInterface;       // Provided interface
        
        unsigned int  mErrorCode;               // Indicates that an error occurred last iteration

        mtsMaxonEPOS *mParent;            // Pointer to parent object

        std::vector<void*> mHandles;

        // Move joint to specified position
        //  servo_jp:  uses Position Tracking mode (PT)
        //  move_jp:  uses Independent Axis Positioning mode (PA, BG)
        void servo_jp(const prmPositionJointSet &jtpos);
        void move_jp(const prmPositionJointSet &jtpos);
        // Move joint to specified relative position
        void servo_jr(const prmPositionJointSet &jtpos);
        // Move joint at specified velocity
        void servo_jv(const prmVelocityJointSet &jtvel);
        // Hold joint at current position (Stop)
        void hold(void);

        // Enable motor power
        void EnableMotorPower(void);
        // Disable motor power
        void DisableMotorPower(void);

        // Set operating state
        void state_command(const std::string &command);

        bool CheckStateEnabled(const char *cmdName) const;

        // Set this point as home.
        void SetHome(void);

        void SetPositionProfile(const vctDoubleVec & profileVelocity, const vctDoubleVec & profileAcceleration, const vctDoubleVec & profileDeceleration);
    };
    RobotData mRobot;

    void Init();
    void Close();

    void SetupInterfaces();
};

CMN_DECLARE_SERVICES_INSTANTIATION(mtsMaxonEPOS)

#endif
