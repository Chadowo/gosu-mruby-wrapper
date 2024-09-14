MRuby::Build.new do |conf|
  conf.toolchain :gcc

  # Include gems
  conf.gembox File.expand_path('gosu', File.dirname(__FILE__))

  # C compiler settings
  conf.defines << 'MRB_INT64'
  conf.cc.flags += %w[-fpermissive -O2]

  conf.enable_cxx_abi
end
