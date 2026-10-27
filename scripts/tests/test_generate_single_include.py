"""Generator regression tests; no third-party Python dependencies required."""
import importlib.util
from pathlib import Path
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'generate_single_include.py'
spec = importlib.util.spec_from_file_location('heim_amalgamation', SCRIPT)
gen = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gen)


class GeneratorTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.base = Path(self.tmp.name) / 'include'
        (self.base / 'heim' / 'internal').mkdir(parents=True)

    def put(self, name, content):
        path = self.base / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding='utf-8')
        return path

    def test_relative_and_root_relative_includes_and_deduplication(self):
        self.put('heim/internal/value.hpp', '#ifndef VALUE_HPP\n#define VALUE_HPP\nstruct Value {};\n#endif\n')
        self.put('heim/internal/a.hpp', '#ifndef A_HPP\n#define A_HPP\n#include "value.hpp"\n#endif\n')
        root = self.put('heim/public.hpp', '#ifndef PUBLIC_HPP\n#define PUBLIC_HPP\n#include <vector>\n#include "heim/internal/a.hpp"\n#include "heim/internal/value.hpp"\n#endif\n')
        result = gen.Amalgamator(self.base).generate(root)
        self.assertIn('#include <vector>', result)
        self.assertEqual(result.count('struct Value {};'), 1)
        self.assertNotIn('#include "heim/internal/value.hpp"', result)

    def test_determinism(self):
        root = self.put('heim/public.hpp', '#ifndef PUBLIC_HPP\n#define PUBLIC_HPP\n#endif\n')
        amalgamator = gen.Amalgamator(self.base)
        self.assertEqual(amalgamator.generate(root), amalgamator.generate(root))

    def test_missing_local_header_is_an_error(self):
        root = self.put('heim/public.hpp', '#include "lost.hpp"\n')
        with self.assertRaisesRegex(gen.GenerationError, 'unresolved'):
            gen.Amalgamator(self.base).generate(root)

    def test_nested_conditional_include_is_an_error(self):
        self.put('heim/value.hpp', 'int value;\n')
        root = self.put('heim/public.hpp', '#ifndef PUBLIC_HPP\n#define PUBLIC_HPP\n#if defined(X)\n#include "value.hpp"\n#endif\n#endif\n')
        with self.assertRaisesRegex(gen.GenerationError, 'nested #if'):
            gen.Amalgamator(self.base).generate(root)


if __name__ == '__main__':
    unittest.main()
