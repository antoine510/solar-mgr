#pragma once

#include <vector>
#include <influxdb.hpp>

#include "Modules/CurrentSensor.hpp"
#include "Modules/MPPT.hpp"

void sendMPTTs(const influxdb_cpp::server_info& si, const std::vector<MPPT>& mppts);
void sendCurrents(const influxdb_cpp::server_info& si, const CurrentSensor& producers, const CurrentSensor& consumers);

