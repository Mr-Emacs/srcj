project('jit-cc', 'cpp',
    version : '0.1',
    default_options : [
        'warning_level=3',
        'cpp_std=c++20'
    ])

src_files = [
    'main.cpp'
]

inc_dirs = include_directories([
    'frontend'
])

executable('jit-cc',
    src_files,
    include_directories : inc_dirs,
    install : true
)
