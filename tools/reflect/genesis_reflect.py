#!/usr/bin/env python3
"""
Genesis Reflection Tool
=======================

Faz parsing REAL de um header C++ (via libclang, o mesmo front-end do
Clang) procurando por classes/structs anotadas com GENESIS_CLASS() e campos
anotados com GENESIS_PROPERTY(). A partir da AST, gera um arquivo
<Nome>.generated.cpp que registra metadados de tipo (nome totalmente
qualificado, tamanho, propriedades com offset e tipo) em
genesis::core::reflection::TypeRegistry.

Isto NÃO é reflection baseada em macro de registro manual: o programador só
anota a declaração; nome, offset (via offsetof, calculado pelo compilador
real ao compilar o .generated.cpp) e tipo de cada propriedade vêm do AST.

Uso:
    genesis_reflect.py <header> --include-as <caminho-do-include> \
        -o <saida.generated.cpp> [-I <include_dir> ...]
"""
import argparse
import subprocess
import sys
from functools import lru_cache
from pathlib import Path

import clang.cindex as ci

ANNOTATE_CLASS = "genesis::reflect::class"
ANNOTATE_PROPERTY = "genesis::reflect::property"

# Tipos suportados pelo runtime de reflection no MVP. Expandir aqui e em
# genesis/core/reflection/TypeInfo.hpp em conjunto ao adicionar um novo tipo.
TYPE_MAP = {
    "float": "Float",
    "int": "Int32",
    "bool": "Bool",
    "genesis::core::math::Vec3": "Vec3",
}


@lru_cache(maxsize=1)
def get_system_include_dirs():
    """O libclang empacotado via pip não traz os headers padrão do sistema
    (stddef.h, os headers de <cmath>/<vector> etc.) — só o binário do parser.
    Descobrimos os diretórios reais perguntando ao compilador do host (o
    mesmo GCC usado para compilar a engine), em vez de fixar caminhos que
    variariam entre distros/versões."""
    try:
        result = subprocess.run(
            ["g++", "-xc++", "-E", "-v", "/dev/null"],
            capture_output=True, text=True, check=True,
        )
    except (FileNotFoundError, subprocess.CalledProcessError):
        return []

    lines = result.stderr.splitlines()
    dirs = []
    collecting = False
    for line in lines:
        if "#include <...> search starts here" in line:
            collecting = True
            continue
        if "End of search list" in line:
            break
        if collecting:
            dirs.append(line.strip())
    return dirs


def find_annotation(cursor: ci.Cursor):
    for child in cursor.get_children():
        if child.kind == ci.CursorKind.ANNOTATE_ATTR:
            return child.spelling
    return None


def fully_qualified_name(cursor: ci.Cursor) -> str:
    parts = [cursor.spelling]
    parent = cursor.semantic_parent
    while parent is not None and parent.kind in (
        ci.CursorKind.NAMESPACE,
        ci.CursorKind.CLASS_DECL,
        ci.CursorKind.STRUCT_DECL,
    ):
        if parent.spelling:
            parts.append(parent.spelling)
        parent = parent.semantic_parent
    return "::".join(reversed(parts))


def collect_reflected_classes(tu: ci.TranslationUnit, source_path: Path):
    """Percorre a AST e retorna [(nome_qualificado, [(prop_nome, tipo_spelling), ...])]."""
    classes = []

    def walk(cursor: ci.Cursor):
        if cursor.kind in (ci.CursorKind.STRUCT_DECL, ci.CursorKind.CLASS_DECL) and cursor.is_definition():
            decl_file = cursor.location.file
            # Só reflete tipos declarados neste arquivo, não em headers incluídos
            # transitivamente (ex.: não queremos reabrir Vec3 aqui).
            if decl_file is not None and Path(str(decl_file)).resolve() == source_path:
                if find_annotation(cursor) == ANNOTATE_CLASS:
                    props = []
                    for member in cursor.get_children():
                        if member.kind == ci.CursorKind.FIELD_DECL and find_annotation(member) == ANNOTATE_PROPERTY:
                            props.append((member.spelling, member.type.spelling))
                    classes.append((fully_qualified_name(cursor), props))
        for child in cursor.get_children():
            walk(child)

    walk(tu.cursor)
    return classes


def generate_cpp(include_as: str, classes, out_path: Path):
    lines = [
        "// AUTO-GERADO por tools/reflect/genesis_reflect.py — NÃO EDITAR À MÃO.",
        "// Qualquer mudança será perdida na próxima geração.",
        f'#include "{include_as}"',
        '#include "genesis/core/reflection/TypeRegistry.hpp"',
        "",
        "namespace {",
        "using namespace genesis::core::reflection;",
        "",
    ]

    for class_name, props in classes:
        safe_name = class_name.replace("::", "_")
        lines.append(f"struct {safe_name}_ReflectionRegistrar {{")
        lines.append(f"    {safe_name}_ReflectionRegistrar() {{")
        lines.append("        TypeInfo info;")
        lines.append(f'        info.name = "{class_name}";')
        lines.append(f"        info.size = sizeof({class_name});")
        for prop_name, type_spelling in props:
            ptype = TYPE_MAP.get(type_spelling)
            if ptype is None:
                print(
                    f"[genesis_reflect] AVISO: tipo '{type_spelling}' de "
                    f"'{class_name}::{prop_name}' não é suportado pelo reflection "
                    f"system ainda; propriedade ignorada. Tipos suportados: "
                    f"{', '.join(TYPE_MAP)}",
                    file=sys.stderr,
                )
                continue
            lines.append(
                f'        info.properties.push_back(PropertyInfo{{"{prop_name}", '
                f"PropertyType::{ptype}, offsetof({class_name}, {prop_name}), "
                f"sizeof({class_name}::{prop_name})}});"
            )
        lines.append("        TypeRegistry::Instance().Register(std::move(info));")
        lines.append("    }")
        lines.append("};")
        lines.append(f"static {safe_name}_ReflectionRegistrar s_{safe_name}_registrar;")
        lines.append("")

    lines.append("}  // namespace")
    lines.append("")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(description="Genesis Reflection Tool")
    parser.add_argument("header", help="Header a parsear (caminho no disco)")
    parser.add_argument("--include-as", required=True, help='Caminho usado no #include "..." gerado')
    parser.add_argument("-o", "--output", required=True, help="Arquivo .generated.cpp de saída")
    parser.add_argument("-I", "--include-dir", action="append", default=[], help="Diretório de include extra")
    parser.add_argument("--libclang", help="Caminho explícito para libclang.so (opcional)")
    args = parser.parse_args()

    if args.libclang:
        ci.Config.set_library_file(args.libclang)

    header_path = Path(args.header).resolve()

    clang_args = ["-std=c++20", "-x", "c++", "-DGENESIS_REFLECT_TOOL=1"]
    for sys_dir in get_system_include_dirs():
        clang_args.append(f"-isystem{sys_dir}")
    for inc in args.include_dir:
        clang_args.append(f"-I{inc}")

    index = ci.Index.create()
    tu = index.parse(str(header_path), args=clang_args)

    errors = [d for d in tu.diagnostics if d.severity >= ci.Diagnostic.Error]
    if errors:
        for d in errors:
            print(f"[genesis_reflect] ERRO ao parsear {header_path.name}: {d}", file=sys.stderr)
        sys.exit(1)

    classes = collect_reflected_classes(tu, header_path)
    if not classes:
        print(f"[genesis_reflect] AVISO: nenhum GENESIS_CLASS() encontrado em {header_path.name}", file=sys.stderr)

    generate_cpp(args.include_as, classes, Path(args.output))

    total_props = sum(len(props) for _, props in classes)
    print(f"[genesis_reflect] {header_path.name}: {len(classes)} tipo(s), {total_props} propriedade(s) -> {args.output}")


if __name__ == "__main__":
    main()
