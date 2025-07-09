cmake_minimum_required(VERSION 3.16)
project(RailwayAnalysis VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts Sql)

# Set up Qt6
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/datamanager.cpp
    src/station.cpp
    src/train.cpp
    src/passengerflow.cpp
    src/analysisengine.cpp
    src/chartwidget.cpp
    src/tablewidget.cpp
    src/predictionmodel.cpp
    src/clusteringmodel.cpp
)

# Header files
set(HEADERS
    include/mainwindow.h
    include/datamanager.h
    include/station.h
    include/train.h
    include/passengerflow.h
    include/analysisengine.h
    include/chartwidget.h
    include/tablewidget.h
    include/predictionmodel.h
    include/clusteringmodel.h
)

# UI files
set(UI_FILES
    ui/mainwindow.ui
)

# Create executable
add_executable(RailwayAnalysis ${SOURCES} ${HEADERS} ${UI_FILES})

# Link Qt6 libraries
target_link_libraries(RailwayAnalysis 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::Charts 
    Qt6::Sql
)

# Copy data files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}) 