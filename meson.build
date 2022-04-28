project('BreathDet', 'cpp',
  version: '1.0',
  default_options: [
    'cpp_std=c++17'
  ],
)
gtkmm_plplot_dep = dependency('gtkmm-plplot-2.0', version: '>=2.0', method : 'pkg-config')

subdir('src')

executable('breath_det', link_with: breath_plot, dependencies: gtkmm_plplot_dep)
