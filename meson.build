project('test', 'c', version: '0.1.0',
  meson_version: '>= 0.40.0',
)

gtkdep = dependency('gtk+-3.0')
paneldep = dependency('libxfce4panel-2.0')
xfconfdep = dependency('libxfconf-0')

exocsource = find_program('exo-csource')

subdir('src')