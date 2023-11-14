
MRuby::Build.new do |conf|
  conf.toolchain :gcc

  # Include gems
  conf.gembox File.expand_path('gosu', File.dirname(__FILE__))

  # C compiler settings
  conf.cc.defines << 'MRB_INT64'
  conf.cc.defines << 'MRB_USE_FLOAT32'
  conf.cc.defines << '_WIN64'
  conf.cc.flags << '-fpermissive' # Needed for mruby-require

  linker.libraries.push('ws2_32')

  conf.enable_debug
  conf.enable_cxx_abi
end
