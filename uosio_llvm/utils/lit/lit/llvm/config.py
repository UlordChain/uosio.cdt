import os
import platform
import re
import subprocess
import sys

import lit.util
from lit.llvm.subst import FindTool
from lit.llvm.subst import ToolSubst


def binary_feature(on, feature, off_prefix):
    return feature if on else off_prefix + feature


class LLVMConfig(object):

    def __init__(self, lit_config, config):
        self.lit_config = lit_config
        self.config = config

        features = config.available_features

        self.use_lit_shell = False
        # Tweak PATH for Win32 to decide to use bash.exe or not.
        if sys.platform == 'win32':
            # For tests that require Windows to run.
            features.add('system-windows')

            # Seek sane tools in directories and set to $PATH.
            path = self.lit_config.getToolsPath(config.lit_tools_dir,
                                                config.environment['PATH'],
                                                ['cmp.exe', 'grep.exe', 'sed.exe'])
            if path is not None:
                self.with_environment('PATH', path, append_path=True)
            self.use_lit_shell = True

        # Choose between lit's internal shell pipeline runner and a real shell.  If
        # LIT_USE_INTERNAL_SHELL is in the environment, we use that as an override.
        lit_shell_env = os.environ.get('LIT_USE_INTERNAL_SHELL')
        if lit_shell_env:
            self.use_lit_shell = lit.util.pythonize_bool(lit_shell_env)

        if not self.use_lit_shell:
            features.add('shell')

        # Running on Darwin OS
        if platform.system() == 'Darwin':
            # FIXME: lld uses the first, other projects use the second.
            # We should standardize on the former.
            features.add('system-linker-mach-o')
            features.add('system-darwin')
        elif platform.system() == 'Windows':
            # For tests that require Windows to run.
            features.add('system-windows')
        elif platform.system() == "Linux":
            features.add('system-linux')

        # Native compilation: host arch == default triple arch
        # Both of these values should probably be in every site config (e.g. as
        # part of the standard header.  But currently they aren't)
        host_triple = getattr(config, 'host_triple', None)
        target_triple = getattr(config, 'target_triple', None)
        if host_triple and host_triple == target_triple:
            features.add('native')

        # Sanitizers.
        sanitizers = getattr(config, 'llvm_use_sanitizer', '')
        sanitizers = frozenset(x.lower() for x in sanitizers.split(';'))
        features.add(binary_feature('address' in sanitizers, 'asan', 'not_'))
        features.add(binary_feature('memory' in sanitizers, 'msan', 'not_'))
        features.add(binary_feature(
            'undefined' in sanitizers, 'ubsan', 'not_'))

        have_zlib = getattr(config, 'have_zlib', None)
        features.add(binary_feature(have_zlib, 'zlib', 'no'))

        # Check if we should run long running tests.
        long_tests = lit_config.params.get('run_long_tests', None)
        if lit.util.pythonize_bool(long_tests):
            features.add('long_tests')

        if target_triple:
            if re.match(r'^x86_64.*-apple', target_triple):
                host_cxx = getattr(config, 'host_cxx', None)
                if 'address' in sanitizers and self.get_clang_has_lsan(host_cxx, target_triple):
                    self.with_environment(
                        'ASAN_OPTIONS', 'detect_leaks=1', append_path=True)
            if re.match(r'^x86_64.*-linux', target_triple):
                features.add('x86_64-linux')
            if re.match(r'.*-win32$', target_triple):
                features.add('target-windows')

        use_gmalloc = lit_config.params.get('use_gmalloc', None)
        if lit.util.pythonize_bool(use_gmalloc):
            # Allow use of an explicit path for gmalloc library.
            # Will default to '/usr/lib/libgmalloc.dylib' if not set.
            gmalloc_path_str = lit_config.params.get('gmalloc_path',
                                                     '/usr/lib/libgmalloc.dylib')
            if gmalloc_path_str is not None:
                self.with_environment(
                    'DYLD_INSERT_LIBRARIES', gmalloc_path_str)

    def with_environment(self, variable, value, append_path=False):
        if append_path:
            # For paths, we should be able to take a list of them and process all
            # of them.
            paths_to_add = value
            if lit.util.is_string(paths_to_add):
                paths_to_add = [paths_to_add]

            def norm(x):
                return os.path.normcase(os.path.normpath(x))

            current_paths = self.config.environment.get(variable, None)
            if current_paths:
                current_paths = current_paths.split(os.path.pathsep)
                paths = [norm(p) for p in current_paths]
            else:
                paths = []

            # If we are passed a list [a b c], then iterating this list forwards
            # and adding each to the beginning would result in b c a.  So we
            # need to iterate in reverse to end up with the original ordering.
            for p in reversed(paths_to_add):
                # Move it to the front if it already exists, otherwise insert it at the
                # beginning.
                p = norm(p)
                try:
                    paths.remove(p)
                except ValueError:
                    pass
                paths = [p] + paths
            value = os.pathsep.join(paths)
        self.config.environment[variable] = value

    def with_system_environment(self, variables, append_path=False):
        if lit.util.is_string(variables):
            variables = [variables]
        for v in variables:
            value = os.environ.get(v)
            if value:
                self.with_environment(v, value, append_path)

    def clear_environment(self, variables):
        for name in variables:
            if name in self.config.environment:
                del self.config.environment[name]

    def get_process_output(self, command):
        try:
            cmd = subprocess.Popen(
                command, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, env=self.config.environment)
            stdout, stderr = cmd.communicate()
            stdout = lit.util.to_string(stdout)
            stderr = lit.util.to_string(stderr)
            return (stdout, stderr)
        except OSError:
            self.lit_config.fatal('Could not run process %s' % command)

    def feature_config(self, features):
        # Ask llvm-config about the specified feature.
        arguments = [x for (x, _) in features]
        config_path = os.path.join(self.config.llvm_tools_dir, 'llvm-config')

        output, _ = self.get_process_output([config_path] + arguments)
        lines = output.split('\n')

        for (feature_line, (_, patterns)) in zip(lines, features):
            # We should have either a callable or a dictionary.  If it's a
            # dictionary, grep each key against the output and use the value if
            # it matches.  If it's a callable, it does the entire translation.
            if callable(patterns):
                features_to_add = patterns(feature_line)
                self.config.available_features.update(features_to_add)
            else:
                for (re_pattern, feature) in patterns.items():
                    if re.search(re_pattern, feature_line):
                        self.config.available_features.add(feature)

    # Note that when substituting %clang_cc1 also fill in the include directory of
    # the builtin headers. Those are part of even a freestanding environment, but
    # Clang relies on the driver to locate them.
    def get_clang_builtin_include_dir(self, clang):
        # FIXME: Rather than just getting the version, we should have clang print
        # out its resource dir here in an easy to scrape form.
        clang_dir, _ = self.get_process_output(
            [clang, '-print-file-name=include'])

        if not clang_dir:
            self.lit_config.fatal(
                "Couldn't find the include dir for Clang ('%s')" % clang)

        clang_dir = clang_dir.strip()
        if sys.platform in ['win32'] and not self.use_lit_shell:
            # Don't pass dosish path separator to msys bash.exe.
            clang_dir = clang_dir.replace('\\', '/')
        # Ensure the result is an ascii string, across Python2.5+ - Python3.
        return clang_dir

    # On macOS, LSan is only supported on clang versions 5 and higher
    def get_clang_has_lsan(self, clang, triple):
        if not clang:
            self.lit_config.warning(
                'config.host_cxx is unset but test suite is configured to use sanitizers.')
            return False

        clang_binary = clang.split()[0]
        version_string, _ = self.get_process_output(
            [clang_binary, '--version'])
        if not 'clang' in version_string:
            self.lit_config.warning(
                "compiler '%s' does not appear to be clang, " % clang_binary +
                'but test suite is configured to use sanitizers.')
            return False

        if re.match(r'.*-linux', triple):
            return True

        if re.match(r'^x86_64.*-apple', triple):
            version_regex = re.search(r'version ([0-9]+)\.([0-9]+).([0-9]+)', version_string)
            major_version_number = int(version_regex.group(1))
            minor_version_number = int(version_regex.group(2))
            patch_version_number = int(version_regex.group(3))
            if 'Apple LLVM' in version_string:
                # Apple LLVM doesn't yet support LSan
                return False
            else:
                return major_version_number >= 5

        return False

    def make_itanium_abi_triple(self, triple):
        m = re.match(r'(\w+)-(\w+)-(\w+)', triple)
        if not m:
            self.lit_config.fatal(
                "Could not turn '%s' into Itanium ABI triple" % triple)
        if m.group(3).lower() != 'win32':
            # All non-win32 triples use the Itanium ABI.
            return triple
        return m.group(1) + '-' + m.group(2) + '-mingw32'

    def make_msabi_triple(self, triple):
        m = re.match(r'(\w+)-(\w+)-(\w+)', triple)
        if not m:
            self.lit_config.fatal(
                "Could not turn '%s' into MS ABI triple" % triple)
        isa = m.group(1).lower()
        vendor = m.group(2).lower()
        os = m.group(3).lower()
        if os == 'win32':
            # If the OS is win32, we're done.
            return triple
        if isa.startswith('x86') or isa == 'amd64' or re.match(r'i\d86', isa):
            # For x86 ISAs, adjust the OS.
            return isa + '-' + vendor + '-win32'
        # -win32 is not supported for non-x86 targets; use a default.
        return 'i686-pc-win32'

    def add_tool_substitutions(self, tools, search_dirs=None):
        if not search_dirs:
            search_dirs = [self.config.llvm_tools_dir]

        if lit.util.is_string(search_dirs):
            search_dirs = [search_dirs]

        tools = [x if isinstance(x, ToolSubst) else ToolSubst(x)
                 for x in tools]

        search_dirs = os.pathsep.join(search_dirs)
        substitutions = []

        for tool in tools:
            match = tool.resolve(self, search_dirs)

            # Either no match occurred, or there was an unresolved match that
            # is ignored.
            if not match:
                continue

            subst_key, tool_pipe, command = match

            # An unresolved match occurred that can't be ignored.  Fail without
            # adding any of the previously-discovered substitutions.
            if not command:
                return False

            substitutions.append((subst_key, tool_pipe + command))

        self.config.substitutions.extend(substitutions)
        return True

    def use_default_substitutions(self):
        tool_patterns = [
            ToolSubst('FileCheck', unresolved='fatal'),
            # Handle these specially as they are strings searched for during testing.
            ToolSubst(r'\| \bcount\b', command=FindTool(
                'count'), verbatim=True, unresolved='fatal'),
            ToolSubst(r'\| \bnot\b', command=FindTool('not'), verbatim=True, unresolved='fatal')]

        self.config.substitutions.append(('%python', sys.executable))
        self.add_tool_substitutions(
            tool_patterns, [self.config.llvm_tools_dir])

    def use_llvm_tool(self, name, search_env=None, required=False, quiet=False):
        """Find the executable program 'name', optionally using the specified
        environment variable as an override before searching the
        configuration's PATH."""
        # If the override is specified in the environment, use it without
        # validation.
        if search_env:
            tool = self.config.environment.get(search_env)
            if tool:
                return tool

        # Otherwise look in the path.
        tool = lit.util.which(name, self.config.environment['PATH'])

        if required and not tool:
            message = "couldn't find '{}' program".format(name)
            if search_env:
                message = message + \
                    ', try setting {} in your environment'.format(search_env)
            self.lit_config.fatal(message)

        if tool:
            tool = os.path.normpath(tool)
            if not self.lit_config.quiet and not quiet:
                self.lit_config.note('using {}: {}'.format(name, tool))
        return tool

    def use_clang(self, required=True):
        """Configure the test suite to be able to invoke clang.

        Sets up some environment variables important to clang, locates a
        just-built or installed clang, and add a set of standard
        substitutions useful to any test suite that makes use of clang.

        """
        # Clear some environment variables that might affect Clang.
        #
        # This first set of vars are read by Clang, but shouldn't affect tests
        # that aren't specifically looking for these features, or are required
        # simply to run the tests at all.
        #
        # FIXME: Should we have a tool that enforces this?

        # safe_env_vars = ('TMPDIR', 'TEMP', 'TMP', 'USERPROFILE', 'PWD',
        #                  'MACOSX_DEPLOYMENT_TARGET', 'IPHONUOS_DEPLOYMENT_TARGET',
        #                  'VCINSTALLDIR', 'VC100COMNTOOLS', 'VC90COMNTOOLS',
        #                  'VC80COMNTOOLS')
        possibly_dangerous_env_vars = ['COMPILER_PATH', 'RC_DEBUG_OPTIONS',
                                       'CINDEXTEST_PREAMBLE_FILE', 'LIBRARY_PATH',
                                       'CPATH', 'C_INCLUDE_PATH', 'CPLUS_INCLUDE_PATH',
                                       'OBJC_INCLUDE_PATH', 'OBJCPLUS_INCLUDE_PATH',
                                       'LIBCLANG_TIMING', 'LIBCLANG_OBJTRACKING',
                                       'LIBCLANG_LOGGING', 'LIBCLANG_BGPRIO_INDEX',
                                       'LIBCLANG_BGPRIO_EDIT', 'LIBCLANG_NOTHREADS',
                                       'LIBCLANG_RESOURCE_USAGE',
                                       'LIBCLANG_CODE_COMPLETION_LOGGING']
        # Clang/Win32 may refer to %INCLUDE%. vsvarsall.bat sets it.
        if platform.system() != 'Windows':
            possibly_dangerous_env_vars.append('INCLUDE')

        self.clear_environment(possibly_dangerous_env_vars)

        # Tweak the PATH to include the tools dir and the scripts dir.
        # Put Clang first to avoid LLVM from overriding out-of-tree clang builds.
        possible_paths = ['clang_tools_dir', 'llvm_tools_dir']
        paths = [getattr(self.config, pp) for pp in possible_paths
                 if getattr(self.config, pp, None)]
        self.with_environment('PATH', paths, append_path=True)

        paths = [self.config.llvm_shlib_dir, self.config.llvm_libs_dir]
        self.with_environment('LD_LIBRARY_PATH', paths, append_path=True)

        # Discover the 'clang' and 'clangcc' to use.

        self.config.clang = self.use_llvm_tool(
            'clang', search_env='CLANG', required=required)

        self.config.substitutions.append(
            ('%llvmshlibdir', self.config.llvm_shlib_dir))
        self.config.substitutions.append(
            ('%pluginext', self.config.llvm_plugin_ext))

        builtin_include_dir = self.get_clang_builtin_include_dir(self.config.clang)
        tool_substitutions = [
            ToolSubst('%clang', command=self.config.clang),
            ToolSubst('%clang_analyze_cc1', command='%clang_cc1', extra_args=['-analyze', '%analyze']),
            ToolSubst('%clang_cc1', command=self.config.clang, extra_args=['-cc1', '-internal-isystem', builtin_include_dir, '-nostdsysteminc']),
            ToolSubst('%clang_cpp', command=self.config.clang, extra_args=['--driver-mode=cpp']),
            ToolSubst('%clang_cl', command=self.config.clang, extra_args=['--driver-mode=cl']),
            ToolSubst('%clangxx', command=self.config.clang, extra_args=['--driver-mode=g++']),
            ]
        self.add_tool_substitutions(tool_substitutions)

        self.config.substitutions.append(('%itanium_abi_triple',
                                          self.make_itanium_abi_triple(self.config.target_triple)))
        self.config.substitutions.append(('%ms_abi_triple',
                                          self.make_msabi_triple(self.config.target_triple)))
        self.config.substitutions.append(
            ('%resource_dir', builtin_include_dir))

        # The host triple might not be set, at least if we're compiling clang from
        # an already installed llvm.
        if self.config.host_triple and self.config.host_triple != '@LLVM_HOST_TRIPLE@':
            self.config.substitutions.append(('%target_itanium_abi_host_triple',
                                              '--target=%s' % self.make_itanium_abi_triple(self.config.host_triple)))
        else:
            self.config.substitutions.append(
                ('%target_itanium_abi_host_triple', ''))

        self.config.substitutions.append(
            ('%src_include_dir', self.config.clang_src_dir + '/include'))

        # FIXME: Find nicer way to prohibit this.
        self.config.substitutions.append(
            (' clang ', """*** Do not use 'clang' in tests, use '%clang'. ***"""))
        self.config.substitutions.append(
            (' clang\+\+ ', """*** Do not use 'clang++' in tests, use '%clangxx'. ***"""))
        self.config.substitutions.append(
            (' clang-cc ',
             """*** Do not use 'clang-cc' in tests, use '%clang_cc1'. ***"""))
        self.config.substitutions.append(
            (' clang -cc1 -analyze ',
             """*** Do not use 'clang -cc1 -analyze' in tests, use '%clang_analyze_cc1'. ***"""))
        self.config.substitutions.append(
            (' clang -cc1 ',
             """*** Do not use 'clang -cc1' in tests, use '%clang_cc1'. ***"""))
        self.config.substitutions.append(
            (' %clang-cc1 ',
             """*** invalid substitution, use '%clang_cc1'. ***"""))
        self.config.substitutions.append(
            (' %clang-cpp ',
             """*** invalid substitution, use '%clang_cpp'. ***"""))
        self.config.substitutions.append(
            (' %clang-cl ',
             """*** invalid substitution, use '%clang_cl'. ***"""))

    def use_lld(self, required=True):
        """Configure the test suite to be able to invoke lld.

        Sets up some environment variables important to lld, locates a
        just-built or installed lld, and add a set of standard
        substitutions useful to any test suite that makes use of lld.

        """
        # Tweak the PATH to include the tools dir
        tool_dirs = [self.config.llvm_tools_dir]
        lib_dirs = [self.config.llvm_libs_dir]
        lld_tools_dir = getattr(self.config, 'lld_tools_dir', None)
        lld_libs_dir = getattr(self.config, 'lld_libs_dir', None)

        if lld_tools_dir:
            tool_dirs = tool_dirs + [lld_tools_dir]
        if lld_libs_dir:
            lib_dirs = lib_dirs + [lld_libs_dir]

        self.with_environment('PATH', tool_dirs, append_path=True)
        self.with_environment('LD_LIBRARY_PATH', lib_dirs, append_path=True)

        tool_patterns = ['lld', 'ld.lld', 'lld-link', 'ld64.lld', 'wasm-ld']

        self.add_tool_substitutions(tool_patterns, tool_dirs)