
MRuby::Build.new do |conf|
  conf.toolchain

  # Include gems
  conf.gembox File.expand_path('gosu', File.dirname(__FILE__))

  # C compiler settings
  conf.cc.defines = %w(MRB_USE_DEBUG_HOOK MRB_INT64)
  conf.cc.flags << '-fpermissive' # Needed for mruby-require

  conf.enable_debug
  conf.enable_cxx_abi
end