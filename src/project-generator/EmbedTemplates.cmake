# Reads one template and stores it as a C++ raw string literal in
# `<variable>` in the caller's scope.
function(juno_read_template path variable)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Missing project template: ${path}")
    endif()

    file(READ "${path}" contents)

    # Raw strings need a delimiter that does not occur in the contents.
    set(delimiter "JUNO_PL_TEMPLATE")
    while(contents MATCHES "\\)${delimiter}\"")
        string(APPEND delimiter "_")
    endwhile()

    set(${variable} "R\"${delimiter}(${contents})${delimiter}\""
        PARENT_SCOPE)
endfunction()

# Embeds the project templates into a generated header so the compiler does
# not need a templates directory at runtime.
function(juno_embed_templates template_dir output)
    juno_read_template("${template_dir}/main.juno" main_template)
    juno_read_template("${template_dir}/config.windows.json"
                       config_windows_template)
    juno_read_template("${template_dir}/config.linux.json"
                       config_linux_template)

    file(WRITE "${output}"
"#pragma once
#include <string_view>

namespace JunoPL::EmbeddedTemplates {
inline constexpr std::string_view main = ${main_template};
inline constexpr std::string_view configWindows = ${config_windows_template};
inline constexpr std::string_view configLinux = ${config_linux_template};
} // namespace JunoPL::EmbeddedTemplates
")
endfunction()
