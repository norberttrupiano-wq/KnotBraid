cmake_minimum_required(VERSION 3.16)

project(LogiKnotting VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport)

add_executable(LogiKnotting
    src/Main.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/WorkspaceScene.h src/ui/WorkspaceScene.cpp
    src/ui/WorkspaceView.h src/ui/WorkspaceView.cpp
    src/ui/Animate.h src/ui/Animate.cpp
    src/core/GridConfig.h
    src/core/Units.h
    src/core/SnapEngine.h src/core/SnapEngine.cpp
    src/core/CylinderTopology.h src/core/CylinderTopology.cpp
    src/model/WorkspaceModel.h src/model/WorkspaceModel.cpp
    src/domain/TopologyTypes.h
    src/domain/TopologyStore.h src/domain/TopologyStore.cpp
    src/tools/ToolPoint.h src/tools/ToolPoint.cpp
    src/render/SegmentBuilder.h src/render/SegmentBuilder.cpp
    src/render/Segment.h
    src/render/Crossing.h
    src/render/CrossingBuilder.h src/render/CrossingBuilder.cpp
    src/render/PointItem.h src/render/PointItem.cpp
    src/render/SegmentItem.h src/render/SegmentItem.cpp
    src/render/CrossingMaskItem.h src/render/CrossingMaskItem.cpp
    src/render/SnapHaloItem.h src/render/SnapHaloItem.cpp
    src/core/RibbonMath.h
    src/core/Ribbon.h
    src/core/RibbonMath.h src/core/RibbonMath.cpp
    src/core/DirectionVector.h
    resources/resources.qrc

)

target_link_libraries(LogiKnotting PRIVATE Qt6::Widgets Qt6::PrintSupport)

target_include_directories(LogiKnotting PRIVATE src)



