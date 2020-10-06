project('lighter', 'c',
	default_options : [
		'warning_level=3',
		'c_std=c18'
	]
)

# Get math library
cc = meson.get_compiler('c')
qlibc_deps = [
	cc.find_library('qlibc', required : false),
	cc.find_library('qlibcext', required : false),
	cc.find_library('pthread', required : false),
	cc.find_library('ssl', required : false),
]
m_dep = cc.find_library('m', required : false)

# Strip binary, optimize it and make it position independent

# Define security flags
security_link_flags = [
	'-Wl,-z,relro',
	'-Wl,-z,now',
]

security_cc_flags = [
	'-fstack-protector',
	'-fstack-protector-strong',
	'--param=ssp-buffer-size=4',
]
add_project_arguments(cc.get_supported_arguments(security_cc_flags), language : 'c')
add_project_link_arguments(cc.get_supported_link_arguments(security_link_flags), language : 'c')

executable('lighter', 'main.c', dependencies : [ m_dep, qlibc_deps ], install : true)
