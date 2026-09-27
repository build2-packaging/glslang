#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/build_info.h>

#if ENABLE_OPT
#  include <string>
#  include <sstream>
#  include <glslang/SPIRV/SpvTools.h>
#endif

#undef NDEBUG
#include <cassert>
#include <vector>

int main ()
{
  glslang::Version v = glslang::GetVersion ();
  assert (v.major == GLSLANG_VERSION_MAJOR);
  assert (v.minor == GLSLANG_VERSION_MINOR);
  assert (v.patch == GLSLANG_VERSION_PATCH);

  assert (glslang::InitializeProcess ());

  const char* src =
    "#version 450\n"
    "void main () {}\n";

  glslang::TShader shader (EShLangVertex);
  shader.setStrings (&src, 1);
  shader.setEnvInput (glslang::EShSourceGlsl,
                      EShLangVertex,
                      glslang::EShClientVulkan,
                      100);
  shader.setEnvClient (glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
  shader.setEnvTarget (glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

  const TBuiltInResource* resources = GetDefaultResources ();
  assert (resources != nullptr);

  EShMessages messages = static_cast<EShMessages> (
    EShMsgSpvRules | EShMsgVulkanRules);
  assert (shader.parse (resources, 450, false, messages));

  glslang::TProgram program;
  program.addShader (&shader);
  assert (program.link (messages));

  std::vector<unsigned int> spirv;
  glslang::GlslangToSpv (*program.getIntermediate (EShLangVertex), spirv);
  assert (!spirv.empty ());

#if ENABLE_OPT
  glslang::SpvOptions options;
  options.disableOptimizer = false;

  std::vector<unsigned int> opt;
  glslang::GlslangToSpv (*program.getIntermediate (EShLangVertex), opt, &options);
  assert (!opt.empty ());

  std::ostringstream os;
  glslang::SpirvToolsDisassemble (os, opt);
  assert (os.str ().find ("OpEntryPoint") != std::string::npos);
#endif

  glslang::FinalizeProcess ();
}
