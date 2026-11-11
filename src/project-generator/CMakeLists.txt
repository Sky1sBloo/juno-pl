set(JUNO_PL_TEMPLATE_DIR "${PROJECT_SOURCE_DIR}/templates")
set(JUNO_PL_EMBEDDED_TEMPLATES
    "${CMAKE_CURRENT_BINARY_DIR}/EmbeddedTemplates.hpp")

# Reconfigure the build (and therefore regenerate the embedded header) when
# a template changes.
set_property(
  DIRECTORY
  APPEND
  PROPERTY CMAKE_CONFIGURE_DEPENDS
           "${JUNO_PL_TEMPLATE_DIR}/main.juno"
           "${JUNO_PL_TEMPLATE_DIR}/config.windows.json"
           "${JUNO_PL_TEMPLATE_DIR}/config.linux.json")

include("${CMAKE_CURRENT_SOURCE_DIR}/EmbedTemplates.cmake")
juno_embed_templates("${JUNO_PL_TEMPLATE_DIR}"
                     "${JUNO_PL_EMBEDDED_TEMPLATES}")

add_library(juno-pl-projectgenerator)
target_sources(
  juno-pl-projectgenerator
  PUBLIC
  FILE_SET CXX_MODULES FILES ProjectGenerator.cppm
  PRIVATE ProjectGenerator.cpp)

target_include_directories(juno-pl-projectgenerator
                           PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

target_compile_features(juno-pl-projectgenerator PUBLIC cxx_std_23)
