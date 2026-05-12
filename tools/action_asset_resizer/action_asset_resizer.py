"""
DesktopPet Action Asset Resizer

独立工具：读取 pet.json 获取 displaySize，批量缩放动作目录中的图片帧。
与 DesktopPet 主程序解耦，不修改任何 C++ 代码或配置文件。
"""

import json
import os
import shutil
import sys
import time
from pathlib import Path

from PIL import Image
from PySide6.QtCore import Qt, QMimeData, QUrl
from PySide6.QtGui import QDragEnterEvent, QDropEvent, QFont
from PySide6.QtWidgets import (
    QApplication,
    QButtonGroup,
    QCheckBox,
    QFileDialog,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QRadioButton,
    QSpinBox,
    QTextEdit,
    QVBoxLayout,
    QWidget,
)

SUPPORTED_EXTENSIONS = {".png", ".jpg", ".jpeg", ".webp", ".bmp"}
GIF_EXTENSION = ".gif"

# 统一控件尺寸
INPUT_HEIGHT = 36
BTN_SELECT_WIDTH = 110
BTN_START_HEIGHT = 40
BTN_START_WIDTH = 160


class DropLineEdit(QLineEdit):
    """支持拖拽路径的 QLineEdit"""

    def __init__(self, placeholder: str = "", parent=None):
        super().__init__(parent)
        self.setPlaceholderText(placeholder)
        self.setAcceptDrops(True)
        self.setFixedHeight(INPUT_HEIGHT)

    def dragEnterEvent(self, event: QDragEnterEvent):
        if event.mimeData().hasUrls():
            event.acceptProposedAction()

    def dropEvent(self, event: QDropEvent):
        urls = event.mimeData().urls()
        if urls:
            path = urls[0].toLocalFile()
            self.setText(path)


def make_select_button(text: str) -> QPushButton:
    """创建统一尺寸的选择按钮。"""
    btn = QPushButton(text)
    btn.setFixedSize(BTN_SELECT_WIDTH, INPUT_HEIGHT)
    return btn


def make_group_box(title: str) -> QGroupBox:
    """创建统一样式的分组框。"""
    box = QGroupBox(title)
    box.setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  border: 1px solid #C0C0C0;"
        "  border-radius: 4px;"
        "  margin-top: 8px;"
        "  padding-top: 14px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 4px;"
        "}"
    )
    return box


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("DesktopPet Action Asset Resizer")
        self.setMinimumSize(900, 680)
        self.setAcceptDrops(True)

        central = QWidget()
        self.setCentralWidget(central)
        layout = QVBoxLayout(central)
        layout.setContentsMargins(16, 12, 16, 12)
        layout.setSpacing(10)

        # --- pet.json ---
        pet_group = make_group_box("pet.json")
        pet_layout = QHBoxLayout(pet_group)
        pet_layout.setContentsMargins(10, 8, 10, 8)
        self.pet_path_edit = DropLineEdit("拖拽或选择 pet.json ...")
        pet_btn = make_select_button("选择...")
        pet_btn.clicked.connect(self.choose_pet_json)
        pet_layout.addWidget(self.pet_path_edit, 1)
        pet_layout.addWidget(pet_btn)
        layout.addWidget(pet_group)

        # --- 目标尺寸 ---
        size_group = make_group_box("目标尺寸（从 pet.json 自动读取，可手动修改）")
        size_layout = QHBoxLayout(size_group)
        size_layout.setContentsMargins(10, 8, 10, 8)
        size_layout.setSpacing(8)
        lbl_w = QLabel("Width:")
        lbl_w.setFixedWidth(52)
        size_layout.addWidget(lbl_w)
        self.width_spin = QSpinBox()
        self.width_spin.setRange(1, 8192)
        self.width_spin.setValue(200)
        self.width_spin.setFixedHeight(INPUT_HEIGHT)
        self.width_spin.setFixedWidth(90)
        size_layout.addWidget(self.width_spin)
        lbl_h = QLabel("Height:")
        lbl_h.setFixedWidth(52)
        size_layout.addWidget(lbl_h)
        self.height_spin = QSpinBox()
        self.height_spin.setRange(1, 8192)
        self.height_spin.setValue(200)
        self.height_spin.setFixedHeight(INPUT_HEIGHT)
        self.height_spin.setFixedWidth(90)
        size_layout.addWidget(self.height_spin)
        self.size_info_label = QLabel("")
        self.size_info_label.setStyleSheet("color: #666;")
        size_layout.addWidget(self.size_info_label, 1)
        layout.addWidget(size_group)

        # --- 动作目录 ---
        action_group = make_group_box("动作目录")
        action_layout = QHBoxLayout(action_group)
        action_layout.setContentsMargins(10, 8, 10, 8)
        self.action_dir_edit = DropLineEdit("拖拽或选择动作目录 ...")
        action_btn = make_select_button("选择...")
        action_btn.clicked.connect(self.choose_action_dir)
        action_layout.addWidget(self.action_dir_edit, 1)
        action_layout.addWidget(action_btn)
        layout.addWidget(action_group)

        # --- 启用备份 + 备份目录 ---
        backup_group = make_group_box("备份设置")
        backup_outer = QVBoxLayout(backup_group)
        backup_outer.setContentsMargins(10, 8, 10, 8)
        backup_outer.setSpacing(6)

        self.backup_checkbox = QCheckBox("处理前备份原动作目录（推荐）")
        self.backup_checkbox.setChecked(True)
        self.backup_checkbox.toggled.connect(self.on_backup_toggled)
        backup_outer.addWidget(self.backup_checkbox)

        backup_row = QHBoxLayout()
        backup_row.setSpacing(8)
        self.backup_dir_edit = DropLineEdit("自动生成，也可自定义 ...")
        self.backup_btn = make_select_button("选择...")
        self.backup_btn.clicked.connect(self.choose_backup_dir)
        backup_row.addWidget(self.backup_dir_edit, 1)
        backup_row.addWidget(self.backup_btn)
        backup_outer.addLayout(backup_row)
        layout.addWidget(backup_group)

        # --- 缩放方式 ---
        scale_group = make_group_box("缩放方式")
        scale_layout = QHBoxLayout(scale_group)
        scale_layout.setContentsMargins(10, 8, 10, 8)
        self.scale_radio_group = QButtonGroup(self)
        self.radio_keep_ratio = QRadioButton("保持比例 + 居中填充（推荐）")
        self.radio_stretch = QRadioButton("直接拉伸")
        self.radio_keep_ratio.setChecked(True)
        self.scale_radio_group.addButton(self.radio_keep_ratio, 0)
        self.scale_radio_group.addButton(self.radio_stretch, 1)
        scale_layout.addWidget(self.radio_keep_ratio)
        scale_layout.addWidget(self.radio_stretch)
        scale_layout.addStretch()
        layout.addWidget(scale_group)

        # --- 操作按钮 ---
        btn_layout = QHBoxLayout()
        btn_layout.addStretch()
        self.start_btn = QPushButton("开始处理")
        self.start_btn.setFixedSize(BTN_START_WIDTH, BTN_START_HEIGHT)
        self.start_btn.setStyleSheet(
            "QPushButton {"
            "  font-weight: bold;"
            "  font-size: 13px;"
            "  border: 1px solid #4A90D9;"
            "  border-radius: 4px;"
            "  background: #4A90D9;"
            "  color: white;"
            "}"
            "QPushButton:hover { background: #3A7BC8; }"
            "QPushButton:pressed { background: #2A6BB8; }"
        )
        self.start_btn.clicked.connect(self.start_processing)
        btn_layout.addWidget(self.start_btn)
        btn_layout.addStretch()
        layout.addLayout(btn_layout)

        # --- 日志 ---
        log_label = QLabel("处理日志")
        log_label.setStyleSheet("font-weight: bold; color: #444;")
        layout.addWidget(log_label)
        self.log_area = QTextEdit()
        self.log_area.setReadOnly(True)
        self.log_area.setMinimumHeight(160)
        log_font = QFont("Consolas", 10)
        log_font.setStyleHint(QFont.Monospace)
        self.log_area.setFont(log_font)
        self.log_area.setStyleSheet(
            "QTextEdit {"
            "  background: #FAFAFA;"
            "  border: 1px solid #D0D0D0;"
            "  border-radius: 3px;"
            "  padding: 4px;"
            "}"
        )
        layout.addWidget(self.log_area, 1)

        # --- 信号连接 ---
        self.pet_path_edit.textChanged.connect(self.on_pet_path_changed)
        self.action_dir_edit.textChanged.connect(self.on_action_dir_changed)

    # ---------- 文件选择 ----------

    def choose_pet_json(self):
        path, _ = QFileDialog.getOpenFileName(self, "选择 pet.json", "", "JSON (*.json)")
        if path:
            self.pet_path_edit.setText(path)

    def choose_action_dir(self):
        path = QFileDialog.getExistingDirectory(self, "选择动作目录")
        if path:
            self.action_dir_edit.setText(path)

    def choose_backup_dir(self):
        path = QFileDialog.getExistingDirectory(self, "选择备份目录")
        if path:
            self.backup_dir_edit.setText(path)

    # ---------- 拖拽 ----------

    def dragEnterEvent(self, event: QDragEnterEvent):
        if event.mimeData().hasUrls():
            event.acceptProposedAction()

    def dropEvent(self, event: QDropEvent):
        urls = event.mimeData().urls()
        if not urls:
            return
        path = urls[0].toLocalFile()
        if os.path.isfile(path) and path.lower().endswith(".json"):
            self.pet_path_edit.setText(path)
        elif os.path.isdir(path):
            self.action_dir_edit.setText(path)

    # ---------- 备份勾选 ----------

    def on_backup_toggled(self, checked: bool):
        self.backup_dir_edit.setEnabled(checked)
        self.backup_btn.setEnabled(checked)

    # ---------- 事件响应 ----------

    def on_pet_path_changed(self, path: str):
        w, h = self.read_display_size(path)
        if w and h:
            self.width_spin.setValue(w)
            self.height_spin.setValue(h)
            self.size_info_label.setText(f"(来自 pet.json: {w}x{h})")
        else:
            self.size_info_label.setText("(未读取到尺寸，请手动输入)")

    def on_action_dir_changed(self, path: str):
        """动作目录变化时，仅在备份目录为空时自动填充。"""
        if not path.strip() or not os.path.isdir(path.strip()):
            return
        if not self.backup_dir_edit.text().strip():
            backup = self.infer_backup_dir(path.strip())
            self.backup_dir_edit.setText(backup)

    @staticmethod
    def read_display_size(pet_json_path: str):
        """从 pet.json 读取 displaySize，返回 (width, height) 或 (None, None)。"""
        try:
            with open(pet_json_path, "r", encoding="utf-8") as f:
                data = json.load(f)
        except Exception:
            return None, None

        ds = data.get("displaySize")
        if isinstance(ds, dict):
            w = ds.get("width") or ds.get("w")
            h = ds.get("height") or ds.get("h")
            if w and h:
                return int(w), int(h)

        w = data.get("displayWidth") or data.get("display_width")
        h = data.get("displayHeight") or data.get("display_height")
        if w and h:
            return int(w), int(h)

        return None, None

    # ---------- 备份目录推断 ----------

    @staticmethod
    def infer_backup_dir(action_dir: str) -> str:
        """
        推断默认备份目录。
        如果 action_dir 的父目录是 actions 且 actions 的父目录是 pets：
          备份到 <project_root>/pets/actions_backup/<actionId>-backup-YYYYMMDD-HHMMSS
        否则：
          备份到 <action_dir_parent>/actions_backup/<actionId>-backup-YYYYMMDD-HHMMSS
        """
        action_path = Path(action_dir).resolve()
        action_id = action_path.name
        timestamp = time.strftime("%Y%m%d-%H%M%S")
        backup_name = f"{action_id}-backup-{timestamp}"

        actions_dir = action_path.parent
        pets_dir = actions_dir.parent
        if actions_dir.name == "actions" and pets_dir.name == "pets":
            # pets 同级创建 actions_backup
            return str(pets_dir / "actions_backup" / backup_name)

        # 无法推断，放到动作目录同级的 actions_backup
        return str(action_path.parent / "actions_backup" / backup_name)

    # ---------- 日志 ----------

    def log(self, msg: str):
        self.log_area.append(msg)

    # ---------- 主处理流程 ----------

    def start_processing(self):
        pet_path = self.pet_path_edit.text().strip()
        action_dir = self.action_dir_edit.text().strip()
        target_w = self.width_spin.value()
        target_h = self.height_spin.value()
        keep_ratio = self.radio_keep_ratio.isChecked()
        backup_enabled = self.backup_checkbox.isChecked()
        backup_dir = self.backup_dir_edit.text().strip()

        # 验证
        if not pet_path or not os.path.isfile(pet_path):
            QMessageBox.warning(self, "错误", "请选择有效的 pet.json 文件。")
            return
        if not action_dir or not os.path.isdir(action_dir):
            QMessageBox.warning(self, "错误", "请选择有效的动作目录。")
            return
        if target_w <= 0 or target_h <= 0:
            QMessageBox.warning(self, "错误", "目标尺寸必须大于 0。")
            return
        if backup_enabled and not backup_dir:
            QMessageBox.warning(self, "错误", "请填写备份目录，或取消勾选备份。")
            return

        action_id = Path(action_dir).name

        # 扫描图片
        image_files = []
        gif_files = []
        for fname in sorted(os.listdir(action_dir)):
            fpath = os.path.join(action_dir, fname)
            if not os.path.isfile(fpath):
                continue
            ext = os.path.splitext(fname)[1].lower()
            if ext in SUPPORTED_EXTENSIONS:
                image_files.append(fpath)
            elif ext == GIF_EXTENSION:
                gif_files.append(fpath)

        if not image_files and not gif_files:
            QMessageBox.information(self, "提示", "动作目录下没有找到可处理的图片文件。")
            return

        # 关闭备份时二次确认
        if not backup_enabled:
            ret = QMessageBox.question(
                self, "确认",
                "你已关闭备份，处理会直接覆盖原图片，且无法恢复。\n是否继续？",
                QMessageBox.Yes | QMessageBox.No,
                QMessageBox.No,
            )
            if ret != QMessageBox.Yes:
                return

        self.log_area.clear()
        self.log("=== DesktopPet Action Asset Resizer ===")
        self.log(f"pet.json: {pet_path}")
        self.log(f"目标尺寸: {target_w} x {target_h}")
        self.log(f"动作目录: {action_dir}")
        self.log(f"ActionId: {action_id}")
        self.log(f"缩放方式: {'保持比例+居中填充' if keep_ratio else '直接拉伸'}")
        self.log(f"备份: {'启用' if backup_enabled else '未启用（直接覆盖）'}")
        if backup_enabled:
            self.log(f"备份目录: {backup_dir}")
        self.log(f"待处理图片: {len(image_files)} 个")

        if gif_files:
            self.log("")
            self.log(f"[跳过] 检测到 {len(gif_files)} 个 GIF 文件:")
            for gf in gif_files:
                self.log(f"  - {os.path.basename(gf)}")
            self.log("  当前版本暂不处理 GIF，请先拆帧或等待后续版本支持。")

        if not image_files:
            self.log("")
            self.log("[中止] 没有可处理的图片（仅有 GIF）。")
            return

        # 备份
        if backup_enabled:
            self.log("")
            self.log("--- 开始备份 ---")
            try:
                # 自动创建 actions_backup 父目录
                backup_parent = Path(backup_dir).parent
                backup_parent.mkdir(parents=True, exist_ok=True)

                final_backup = backup_dir
                if os.path.exists(final_backup):
                    final_backup = backup_dir + "_" + str(int(time.time()))
                    self.log(f"  备份目录已存在，改用: {final_backup}")
                shutil.copytree(action_dir, final_backup)
                self.log(f"  备份完成: {final_backup}")
                backup_dir = final_backup
            except Exception as e:
                self.log(f"  [错误] 备份失败: {e}")
                QMessageBox.critical(self, "错误", f"备份失败，中止处理。\n{e}")
                return
        else:
            self.log("")
            self.log("--- 跳过备份（未启用） ---")

        # 缩放
        self.log("")
        self.log("--- 开始缩放 ---")
        success_count = 0
        fail_count = 0
        skipped_size_count = 0
        for fpath in image_files:
            fname = os.path.basename(fpath)
            try:
                status = self.resize_image(fpath, target_w, target_h, keep_ratio)
                if status == "skipped":
                    self.log(f"  [跳过] {fname} 已是目标尺寸")
                    skipped_size_count += 1
                else:
                    self.log(f"  [OK] {fname}")
                    success_count += 1
            except Exception as e:
                self.log(f"  [失败] {fname}: {e}")
                fail_count += 1

        self.log("")
        self.log("=== 处理完成 ===")
        self.log(f"成功: {success_count}, 失败: {fail_count}, "
                 f"跳过(已是目标尺寸): {skipped_size_count}, 跳过GIF: {len(gif_files)}")
        if backup_enabled:
            self.log(f"备份位于: {backup_dir}")

    @staticmethod
    def resize_image(fpath: str, target_w: int, target_h: int, keep_ratio: bool) -> str:
        """
        缩放单张图片并覆盖原文件。
        返回 "ok" / "skipped"
        """
        ext = os.path.splitext(fpath)[1].lower()

        with Image.open(fpath) as src:
            img = src.copy()

        if img.width == target_w and img.height == target_h:
            return "skipped"

        has_alpha = img.mode in ("RGBA", "LA") or (
            img.mode == "P" and "transparency" in img.info
        )

        if keep_ratio:
            img.thumbnail((target_w, target_h), Image.LANCZOS)
            if ext in (".png", ".webp") and has_alpha:
                canvas = Image.new("RGBA", (target_w, target_h), (0, 0, 0, 0))
                if img.mode != "RGBA":
                    img = img.convert("RGBA")
            else:
                canvas = Image.new("RGB", (target_w, target_h), (255, 255, 255))
                if img.mode != "RGB":
                    img = img.convert("RGB")
            offset_x = (target_w - img.width) // 2
            offset_y = (target_h - img.height) // 2
            canvas.paste(img, (offset_x, offset_y))
            result = canvas
        else:
            result = img.resize((target_w, target_h), Image.LANCZOS)

        save_kwargs = {}
        if ext in (".jpg", ".jpeg"):
            if result.mode != "RGB":
                result = result.convert("RGB")
            save_kwargs["quality"] = 95
        elif ext == ".png":
            save_kwargs["optimize"] = True
        elif ext == ".webp":
            save_kwargs["quality"] = 95

        result.save(fpath, **save_kwargs)
        return "ok"


def main():
    app = QApplication(sys.argv)

    # 统一应用字体，避免默认字体在高 DPI 下异常
    font = QFont("Microsoft YaHei", 10)
    font.setStyleHint(QFont.SansSerif)
    app.setFont(font)

    window = MainWindow()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
