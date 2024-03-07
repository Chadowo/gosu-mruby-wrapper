MRuby::Build.new do |conf|
  conf.toolchain :gcc

  # Include gems
  conf.gembox File.expand_path('gosu', File.dirname(__FILE__))

  # C compiler settings
  conf.cc.defines << %w[MRB_INT64 _WIN64]
  conf.cc.flags << '-fpermissive'

  linker.libraries.push('ws2_32')

  conf.enable_debug
  conf.enable_cxx_abi
end
