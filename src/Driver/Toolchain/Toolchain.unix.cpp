//
// Created by Albert Varaksin on 07/02/2021.
//

fs::path Toolchain::getPath(ToolKind tool) {
    fs::path path;
    switch (tool) {
    case ToolKind::Optimizer:
        if (m_basePath == "") {
            path = "/usr/local/bin/opt";
        } else {
            path = m_basePath / "opt";
        }
        break;
    case ToolKind::Assembler:
        if (m_basePath == "") {
            path = "/usr/local/bin/llc";
        } else {
            path = m_basePath / "llc";
        }
        break;
    case ToolKind::Linker:
        if (m_basePath == "") {
            path = "/usr/bin/ld";
        } else {
            path = m_basePath / "ld";
        }
        break;
    default:
        llvm_unreachable("Invalid ToolKind ID");
    }
    if (!fs::exists(path)) {
        fatalError("ToolKind "_t + path.string() + " not found!");
    }
    return path;
}
