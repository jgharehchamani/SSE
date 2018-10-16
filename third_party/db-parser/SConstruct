import os

env = Environment()

try:
    env.Append(ENV = {'TERM' : os.environ['TERM']}) # Keep our nice terminal environment (like colors ...)
except:
    print("Not running in a terminal")


if FindFile('config.scons', '.'):
    SConscript('config.scons', exports='env')
    
env.Append(CFLAGS=['-std=c99'])
env.Append(CCFLAGS=['-Wall', '-march=native', '-maes', '-fPIC'])
env.Append(CXXFLAGS=['-std=c++11'])


env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME']=1

debug = ARGUMENTS.get('debug', 0)

if int(debug):
    env.Append(CCFLAGS = ['-g','-O'])
else:
	env.Append(CCFLAGS = ['-O2'])



def run_test(target, source, env):
    app = str(source[0].abspath)
    if os.spawnl(os.P_WAIT, app, app)==0:
        return 0
    else:
        return 1

bld = Builder(action = run_test)
env.Append(BUILDERS = {'Test' :  bld})

objects = SConscript('src/build.scons', exports='env', variant_dir='build', duplicate=0)
test_objects = SConscript('test/build.scons', exports='env', variant_dir='build_test', duplicate=0)

Clean(objects, 'build')
Clean(test_objects, 'build_test')

debug_prog = env.Program(['test/main.cpp'] + objects, CPPPATH = ['src'] + env.get('CPPPATH', []))
env.Default(debug_prog)

shared_lib_env = env.Clone();

if env['PLATFORM'] == 'darwin':
    # We have to add '@rpath' to the library install name
    shared_lib_env.Append(LINKFLAGS = ['-install_name', '@rpath/libsse_dbparser.dylib'])
    
library_build_prefix = 'library'
shared_lib = shared_lib_env.SharedLibrary(library_build_prefix+'/lib/sse_dbparser',objects);
static_lib = env.StaticLibrary(library_build_prefix+'/lib/sse_dbparser',objects)

headers = Glob('src/*.h') + Glob('src/json/*.h')
rapidjson = Dir("src/json/rapidjson")

headers_lib = [env.Install(library_build_prefix+'/include/sse/dbparser', headers + [rapidjson])]
# rapidjson_lib = [env.Install(library_build_prefix+'/include/sse/dbparser', rapidjson)]

env.Clean(headers_lib,[library_build_prefix+'/include'])

Alias('headers', headers_lib)
Alias('lib', [shared_lib, static_lib] + headers_lib)
Clean('lib', 'library')
# Alias('lib', [lib_install] + headers_lib)

