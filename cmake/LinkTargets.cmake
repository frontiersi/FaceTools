if(WITH_TESTUTILS)
    target_link_libraries( ${PROJECT_NAME} ${TestUtils_LIBRARIES})
endif()

if(WITH_FACETOOLS)
    target_link_libraries( ${PROJECT_NAME} ${FaceTools_LIBRARIES})
endif()

if(WITH_QTOOLS)
    target_link_libraries( ${PROJECT_NAME} ${QTools_LIBRARIES})
endif()

if(WITH_RPASCALVOC)
    target_link_libraries( ${PROJECT_NAME} ${rPascalVOC_LIBRARIES})
endif()

if(WITH_RLEARNING)
    target_link_libraries( ${PROJECT_NAME} ${rLearning_LIBRARIES})
endif()

if(WITH_R3DVIS)
    target_link_libraries( ${PROJECT_NAME} ${r3dvis_LIBRARIES})
endif()

if(WITH_R3DIO)
    target_link_libraries( ${PROJECT_NAME} ${r3dio_LIBRARIES})
endif()

if(WITH_R3D)
    target_link_libraries( ${PROJECT_NAME} ${r3d_LIBRARIES})
endif()

if(WITH_RNONRIGID)
    target_link_libraries( ${PROJECT_NAME} ${rNonRigid_LIBRARIES})
endif()

if(WITH_RIMG)
    target_link_libraries( ${PROJECT_NAME} ${rimg_LIBRARIES})
endif()

if(WITH_RLIB)
    target_link_libraries( ${PROJECT_NAME} ${rlib_LIBRARIES})
endif()

if(WITH_ASSIMP)
    target_link_libraries( ${PROJECT_NAME} ${ASSIMP_LIBRARIES})
endif()

if(WITH_NANOFLANN)
    target_link_libraries( ${PROJECT_NAME} nanoflann::nanoflann)
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
