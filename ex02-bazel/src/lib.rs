use tree_sitter::{Language, Parser};
use tree_sitter as ts;
extern "C" {
    fn tree_sitter_python() -> ts::Language;
    fn tree_sitter_rust() -> ts::Language;
    fn tree_sitter_go() -> ts::Language;
    fn tree_sitter_javascript() -> ts::Language;
    fn tree_sitter_typescript() -> ts::Language;
  }

fn main() {
    let code = "function test() { return 42; }";

    let language = unsafe { tree_sitter_typescript() };
    let mut parser = Parser::new();
    parser.set_language(language).unwrap();

    let tree = parser.parse(code, None).unwrap();
    let root_node = tree.root_node();

    println!("Root node: {:?}", root_node);
    println!("Root node type: {}", root_node.kind());
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_javascript() {
        let code = "function test() { return 42; }";

        let language = unsafe { tree_sitter_typescript() };
        let mut parser = Parser::new();
        parser.set_language(language).unwrap();

        let tree = parser.parse(code, None).unwrap();
        let root_node = tree.root_node();

        assert_eq!(root_node.kind(), "program");
        assert_eq!(root_node.start_position().row, 0);
        assert_eq!(root_node.start_position().column, 0);
        assert_eq!(root_node.end_position().row, 0);
        assert_eq!(root_node.end_position().column, 30);
    }
}