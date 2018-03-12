// ----------------------------------------------------------------------------
// ToydadInterface.cpp
//
//
// Authors:
// Peter Polidoro polidorop@janelia.hhmi.org
// ----------------------------------------------------------------------------
#include "../ToydadInterface.h"


using namespace toydad_interface;

ToydadInterface::ToydadInterface()
{
}

void ToydadInterface::setup()
{
  // Parent Setup
  SerialInterface::setup();

  // Reset Watchdog
  resetWatchdog();

  // Variable Setup

  // Set Device ID
  modular_server_.setDeviceName(constants::device_name);

  // Add Hardware

  // Interrupts

  // Add Firmware
  modular_server_.addFirmware(constants::firmware_info,
                              properties_,
                              parameters_,
                              functions_,
                              callbacks_);

  // Properties
  modular_server::Property & bauds_property = modular_server_.property(serial_interface::constants::bauds_property_name);
  bauds_property.setDefaultValue(constants::bauds_default);
  bauds_property.setSubset(constants::baud_subset);

  modular_server::Property & formats_property = modular_server_.property(serial_interface::constants::formats_property_name);
  formats_property.setDefaultValue(constants::formats_default);
  formats_property.setSubset(constants::format_ptr_subset);

  modular_server::Property & line_endings_property = modular_server_.property(serial_interface::constants::line_endings_property_name);
  line_endings_property.setDefaultValue(constants::line_endings_default);
  line_endings_property.setSubset(constants::line_ending_ptr_subset);

  modular_server::Property & timeouts_property = modular_server_.property(serial_interface::constants::timeouts_property_name);
  timeouts_property.setDefaultValue(constants::timeouts_default);

  // Parameters

  // Functions
  modular_server::Function & get_detector_info_function = modular_server_.createFunction(constants::get_detector_info_function_name);
  get_detector_info_function.attachFunctor(makeFunctor((Functor0 *)0,*this,&ToydadInterface::getDetectorInfoHandler));
  get_detector_info_function.setResultTypeObject();

  modular_server::Function & get_status_function = modular_server_.createFunction(constants::get_status_function_name);
  get_status_function.attachFunctor(makeFunctor((Functor0 *)0,*this,&ToydadInterface::getStatusHandler));
  get_status_function.setResultTypeString();

  // Callbacks
}

bool ToydadInterface::communicating()
{
  char * detector_name = NULL;
  return getDetectorName(detector_name);
}

bool ToydadInterface::getDetectorName(char * & detector_name)
{
  detector_name = response_data_;
  const char key[] = "DTr";
  bool success = sendCommandGetResponse(key);
  return success;
}

bool ToydadInterface::getHardwareVersion(char * & hardware_version)
{
  hardware_version = response_data_;
  const char key[] = "HWr";
  bool success = sendCommandGetResponse(key);
  return success;
}

bool ToydadInterface::getModel(char * & model)
{
  model = response_data_;
  const char key[] = "MDr";
  bool success = sendCommandGetResponse(key);
  return success;
}

bool ToydadInterface::getFirmwareVersion(char * & firmware_version)
{
  firmware_version = response_data_;
  const char key[] = "SWr";
  bool success = sendCommandGetResponse(key);
  return success;
}

bool ToydadInterface::getSerialNumber(char * & serial_number)
{
  serial_number = response_data_;
  const char key[] = "SNr";
  bool success = sendCommandGetResponse(key);
  return success;
}

bool ToydadInterface::getStatus(Status & status)
{
  const char key[] = "STr";
  bool success = sendCommandGetResponse(key);
  if (!success)
  {
    return success;
  }
  char status_str[constants::STATUS_BUFFER_SIZE];
  memset(status_str,0,constants::STATUS_BUFFER_SIZE);
  strncpy(status_str,(response_data_ + constants::STATUS_OFFSET),constants::STATUS_SIZE);
  status = static_cast<Status>(atoi(response_data_));
  return success;
}

bool ToydadInterface::sendCommandGetResponse(const char key[])
{
  initializeResponse();
  char data[constants::REQUEST_SIZE_MAX];
  data[0] = '\0';
  strcat(data,constants::line_beginning);
  strcat(data,key);
  writeRead(data,response_,constants::RESPONSE_SIZE_MAX);
  strncpy(response_key_,response_,constants::KEY_SIZE);
  bool success = false;
  if (strcmp(key,response_key_) == 0)
  {
    success = true;
  }
  return success;
}

size_t ToydadInterface::getResponseLength()
{
  return strlen(response_);
}

void ToydadInterface::initializeResponse()
{
  memset(response_key_,0,constants::KEY_BUFFER_SIZE);
  response_data_[0] = '\0';
}

// Handlers must be non-blocking (avoid 'delay')
//
// modular_server_.parameter(parameter_name).getValue(value) value type must be either:
// fixed-point number (int, long, etc.)
// floating-point number (float, double)
// bool
// const char *
// ArduinoJson::JsonArray *
// ArduinoJson::JsonObject *
// const ConstantString *
//
// For more info read about ArduinoJson parsing https://github.com/janelia-arduino/ArduinoJson
//
// modular_server_.property(property_name).getValue(value) value type must match the property default type
// modular_server_.property(property_name).setValue(value) value type must match the property default type
// modular_server_.property(property_name).getElementValue(element_index,value) value type must match the property array element default type
// modular_server_.property(property_name).setElementValue(element_index,value) value type must match the property array element default type

void ToydadInterface::getDetectorInfoHandler()
{
  if (!communicating())
  {
    modular_server_.response().returnError(constants::not_communicating_error);
    return;
  }
  modular_server_.response().writeResultKey();

  modular_server_.response().beginObject();

  char * response_data = NULL;

  getDetectorName(response_data);
  modular_server_.response().write(constants::detector_name_constant_string,response_data);

  getHardwareVersion(response_data);
  modular_server_.response().write(constants::hardware_version_constant_string,response_data);

  getModel(response_data);
  modular_server_.response().write(constants::model_constant_string,response_data);

  getFirmwareVersion(response_data);
  modular_server_.response().write(constants::firmware_version_constant_string,response_data);

  getSerialNumber(response_data);
  modular_server_.response().write(constants::serial_number_constant_string,response_data);

  modular_server_.response().endObject();
}

void ToydadInterface::getStatusHandler()
{
  if (!communicating())
  {
    modular_server_.response().returnError(constants::not_communicating_error);
    return;
  }
  modular_server_.response().writeResultKey();

  modular_server_.response().beginObject();

  Status status;
  getStatus(status);
  ConstantString * status_ptr = NULL;
  switch (status)
  {
    case STANDBY:
    {
      status_ptr = &constants::status_standby_constant_string;
      break;
    }
    case LAMP_IGNITION:
    {
      status_ptr = &constants::status_lamp_ignition_constant_string;
      break;
    }
    case MEASUREMENT:
    {
      status_ptr = &constants::status_measurement_constant_string;
      break;
    }
    case AUTOZERO:
    {
      status_ptr = &constants::status_autozero_constant_string;
      break;
    }
    case SCAN_ABS:
    {
      status_ptr = &constants::status_scan_abs_constant_string;
      break;
    }
    case SCAN_ITS:
    {
      status_ptr = &constants::status_scan_its_constant_string;
      break;
    }
    case USER_CALIB:
    {
      status_ptr = &constants::status_user_calib_constant_string;
      break;
    }
    case SELF_TEST:
    {
      status_ptr = &constants::status_self_test_constant_string;
      break;
    }
    case SCAN_ABS_SUBS:
    {
      status_ptr = &constants::status_scan_abs_subs_constant_string;
      break;
    }
  }
  modular_server_.response().write(constants::status_constant_string,status_ptr);

  modular_server_.response().endObject();
}
