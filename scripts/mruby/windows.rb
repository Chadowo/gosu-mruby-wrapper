
MRuby::Build.new do |conf|
  conf.toolchain :gcc

  # Include gems
  conf.gembox File.expand_path('gosu', File.dirname(__FILE__))

  conf.cc.defines << 'MRB_INT64'
  conf.cc.flags << '-fpermissive' # Needed for mruby-require

  linker.libraries.push('ws2_32')

  conf.enable_debug
  conf.enable_cxx_abi
end
