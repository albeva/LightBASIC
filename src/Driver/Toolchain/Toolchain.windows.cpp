//
// Created by Albert Varaksin on 07/02/2021.
//

fs::path Toolchain::getPath(ToolKind tool) const noexcept {
    fs::path path;
    switch (tool) {
    case ToolKind::Optimizer:
        path = m_basePath / "bin" / "opt.exe";
        break;
    case ToolKind::Assembler:
        path = m_basePath / "bin" / "llc.exe";
        break;
    case ToolKind::Linker:
        path = m_basePath / "bin" / "ld.exe";
        break;
    default:
        llvm_unreachable("Invalid ToolKind ID");
    }
    if (!fs::exists(path)) {
        fatalError("Tool '"s + path.string() + "' not found!");
    }
    return path;
}
