gtkmm_plplot_dep = dependency('gtkmm-plplot-2.0', version: '>=2.0',
  method: 'pkg-config')

src_files = [
  'main.cpp',
  'breath_plot.cpp',
  'base.cpp',
  'input_handler.cpp',
  'data_processing.cpp'
]

breath_plot = shared_library('breath_plot',
                         src_files,
                         dependencies: gtkmm_plplot_dep,
                         )
