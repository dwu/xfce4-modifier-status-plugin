gen_properties_dialog = custom_target('gen-dialog',
                            input : ['properties-dialog.glade'],
                            output : ['properties-dialog.h'],
                            command : [exocsource,
                                       '--static',
                                       '--strip-comments',
                                       '--strip-content',
                                       '--name=properties_dialog_ui',
                                       '--output=@OUTPUT0@',
                                       '@INPUT@'])

shared_library(
    'xfce4-modifier-status-plugin',
    ['xfce4-modifier-status-plugin.c', gen_properties_dialog ],
    dependencies: [ gtkdep, paneldep, xfconfdep ],
    install: true,
    install_dir: '/usr/lib/xfce4/panel/plugins')

install_data(
    'xfce4-modifier-status-plugin.desktop',
    install_dir: '/usr/share/xfce4/panel/plugins')
