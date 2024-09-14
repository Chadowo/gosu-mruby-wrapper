MRuby::Build.new do |conf|
  conf.toolchain :visualcpp

  # Include gems
  conf.gembox File.expand_path('gosu', File.dirname(__FILE__))

  conf.defines << 'MRB_INT64'

  linker.libraries.push('ws2_32')

  conf.enable_debug # Won't compile otherwise
  conf.enable_cxx_abi
end
