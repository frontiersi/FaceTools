if(WITH_TESTUTILS)
    target_link_libraries( ${PROJECT_NAME} ${TestUtils_LIBRARIES})
endif()

if(WITH_FACETOOLS)
    target_link_libraries( ${PROJECT_NAME} ${FaceTools_LIBRARIES})
endif()

if(WITH_QTOOLS)
    target_link_libraries( ${PROJECT_NAME} ${QTools_LIBRARIES})
endif()

if(WITH_RVTK)
    target_link_libraries( ${PROJECT_NAME} ${rVTK_LIBRARIES})
endif()

if(WITH_RPASCALVOC)
    target_link_libraries( ${PROJECT_NAME} ${rPascalVOC_LIBRARIES})
endif()

if(WITH_RLEARNING)
    target_link_libraries( ${PROJECT_NAME} ${rLearning_LIBRARIES})
endif()

if(WITH_RMODELIO)
    target_link_libraries( ${PROJECT_NAME} ${rModelIO_LIBRARIES})
endif()

if(WITH_RFEATURES)
    target_link_libraries( ${PROJECT_NAME} ${rFeatures_LIBRARIES})
endif()

if(WITH_RLIB)
    target_link_libraries( ${PROJECT_NAME} ${rlib_LIBRARIES})
endif()

if(WITH_ASSIMP)
    target_link_libraries( ${PROJECT_NAME} ${ASSIMP_LIBRARIES})
endif()

if(WITH_TINYXML)
    target_link_libraries( ${PROJECT_NAME} ${tinyxml_LIBRARY})
endif()

if(WITH_BOOST)
    target_link_libraries( ${PROJECT_NAME} ${Boost_LIBRARIES})
endif()

if(WITH_OPENCV)
    target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS})
endif()

if(WITH_VTK)
    target_link_libraries( ${PROJECT_NAME} ${VTK_LIBRARIES})
endif()

if(WITH_QT)
    #target_link_libraries( ${PROJECT_NAME} Qt5::Widgets Qt5::Charts Qt5::Svg)
    target_link_libraries( ${PROJECT_NAME} ${QT_LIBRARIES})
endif()

if(WITH_CGAL)
    target_link_libraries( ${PROJECT_NAME} ${CGAL_LIBRARIES})
endif()

if(WITH_DLIB)
    target_link_libraries( ${PROJECT_NAME} ${dlib_LIBRARIES})
endif()

if(WITH_QUAZIP)
    target_link_libraries( ${PROJECT_NAME} ${QuaZip_LIBRARIES})
endif()

if(WITH_ZLIB)
    target_link_libraries( ${PROJECT_NAME} ${ZLIB_LIBRARIES})
endif()

if(WITH_LUA)
    target_link_libraries( ${PROJECT_NAME} ${LUA_LIBRARIES})
endif()
