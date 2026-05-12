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
from PySide6.QtGui import QDragEnterEvent, QDropEvent
from PySide6.QtWidgets import (
    QApplication,
    QButtonGroup,
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


class DropLineEdit(QLineEdit):
    """支持拖拽路径的 QLineEdit"""

    def __init__(self, placeholder: str = "", parent=None):
        super().__init__(parent)
        self.setPlaceholderText(placeholder)
        self.setAcceptDrops(True)

    def dragEnterEvent(self, event: QDragEnterEvent):
        if event.mimeData().hasUrls():
            event.acceptProposedAction()

    def dropEvent(self, event: QDropEvent):
        urls = event.mimeData().urls()
        if urls:
            path = urls[0].toLocalFile()
            self.setText(path)


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("DesktopPet Action Asset Resizer")
        self.setMinimumSize(640, 580)
        self.setAcceptDrops(True)

        central = QWidget()
        self.setCentralWidget(central)
        layout = QVBoxLayout(central)

        # --- pet.json ---
        pet_group = QGroupBox("pet.json")
        pet_layout = QHBoxLayout(pet_group)
        self.pet_path_edit = DropLineEdit("拖拽或选择 pet.json ...")
        pet_btn = QPushButton("选择...")
        pet_btn.clicked.connect(self.choose_pet_json)
        pet_layout.addWidget(self.pet_path_edit, 1)
        pet_layout.addWidget(pet_btn)
        layout.addWidget(pet_group)

        # --- 目标尺寸 ---
        size_group = QGroupBox("目标尺寸 (从 pet.json 自动读取，可手动修改)")
        size_layout = QHBoxLayout(size_group)
        size_layout.addWidget(QLabel("Width:"))
        self.width_spin = QSpinBox()
        self.width_spin.setRange(1, 8192)
        self.width_spin.setValue(200)
        size_layout.addWidget(self.width_spin)
        size_layout.addWidget(QLabel("Height:"))
        self.height_spin = QSpinBox()
        self.height_spin.setRange(1, 8192)
        self.height_spin.setValue(200)
        size_layout.addWidget(self.height_spin)
        self.size_info_label = QLabel("")
        size_layout.addWidget(self.size_info_label, 1)
        layout.addWidget(size_group)

        # --- 动作目录 ---
        action_group = QGroupBox("动作目录")
        action_layout = QHBoxLayout(action_group)
        self.action_dir_edit = DropLineEdit("拖拽或选择动作目录 ...")
        action_btn = QPushButton("选择...")
        action_btn.clicked.connect(self.choose_action_dir)
        action_layout.addWidget(self.action_dir_edit, 1)
        action_layout.addWidget(action_btn)
        layout.addWidget(action_group)

        # --- 备份目录 ---
        backup_group = QGroupBox("备份目录 (默认自动推断，可自定义)")
        backup_layout = QHBoxLayout(backup_group)
        self.backup_dir_edit = DropLineEdit("自动生成，也可自定义 ...")
        backup_btn = QPushButton("选择...")
        backup_btn.clicked.connect(self.choose_backup_dir)
        backup_layout.addWidget(self.backup_dir_edit, 1)
        backup_layout.addWidget(backup_btn)
        layout.addWidget(backup_group)

        # --- 缩放方式 ---
        scale_group = QGroupBox("缩放方式")
        scale_layout = QHBoxLayout(scale_group)
        self.scale_radio_group = QButtonGroup(self)
        self.radio_keep_ratio = QRadioButton("保持比例 + 居中填充 (推荐)")
        self.radio_stretch = QRadioButton("直接拉伸")
        self.radio_keep_ratio.setChecked(True)
        self.scale_radio_group.addButton(self.radio_keep_ratio, 0)
        self.scale_radio_group.addButton(self.radio_stretch, 1)
        scale_layout.addWidget(self.radio_keep_ratio)
        scale_layout.addWidget(self.radio_stretch)
        layout.addWidget(scale_group)

        # --- 操作按钮 ---
        btn_layout = QHBoxLayout()
        self.start_btn = QPushButton("开始处理")
        self.start_btn.clicked.connect(self.start_processing)
        btn_layout.addStretch()
        btn_layout.addWidget(self.start_btn)
        btn_layout.addStretch()
        layout.addLayout(btn_layout)

        # --- 日志 ---
        self.log_area = QTextEdit()
        self.log_area.setReadOnly(True)
        layout.addWidget(self.log_area, 1)

        # pet.json 变化时自动读取尺寸
        self.pet_path_edit.textChanged.connect(self.on_pet_path_changed)

        # 动作目录变化时自动填充备份目录
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
            self.try_fill_backup_dir()

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
            self.try_fill_backup_dir()

    # ---------- pet.json 读取 ----------

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

        # 尝试多种可能的字段名
        ds = data.get("displaySize")
        if isinstance(ds, dict):
            w = ds.get("width") or ds.get("w")
            h = ds.get("height") or ds.get("h")
            if w and h:
                return int(w), int(h)

        # 平铺字段
        w = data.get("displayWidth") or data.get("display_width")
        h = data.get("displayHeight") or data.get("display_height")
        if w and h:
            return int(w), int(h)

        return None, None

    # ---------- 备份目录推断 ----------

    def try_fill_backup_dir(self):
        """根据动作目录自动推断备份目录。"""
        action_dir = self.action_dir_edit.text().strip()
        if not action_dir or not os.path.isdir(action_dir):
            return
        if self.backup_dir_edit.text().strip():
            return  # 用户已自定义，不覆盖

        backup = self.infer_backup_dir(action_dir)
        self.backup_dir_edit.setText(backup)

    @staticmethod
    def infer_backup_dir(action_dir: str) -> str:
        """
        推断默认备份目录。
        如果 action_dir 的父目录名是 actions，且 actions 的父目录名是 pets：
          project_root = action_dir.parent.parent.parent  (即 pets 的上级)
          backup_base = project_root.parent                (即项目根目录的上级)
          备份目录 = backup_base / "<actionId>-backup-时间戳"
        否则：
          备份目录 = action_dir.parent / "<actionId>-backup-时间戳"
        """
        action_path = Path(action_dir).resolve()
        action_id = action_path.name
        timestamp = time.strftime("%Y%m%d_%H%M%S")
        backup_name = f"{action_id}-backup-{timestamp}"

        # 检查 .../pets/actions/<actionId> 结构
        actions_dir = action_path.parent  # actions
        pets_dir = actions_dir.parent     # pets
        if actions_dir.name == "actions" and pets_dir.name == "pets":
            project_root = pets_dir.parent
            backup_base = project_root.parent
            return str(backup_base / backup_name)

        # 无法推断，放到动作目录同级
        return str(action_path.parent / backup_name)

    # ---------- 日志 ----------

    def log(self, msg: str):
        self.log_area.append(msg)

    # ---------- 主处理流程 ----------

    def start_processing(self):
        pet_path = self.pet_path_edit.text().strip()
        action_dir = self.action_dir_edit.text().strip()
        backup_dir = self.backup_dir_edit.text().strip()
        target_w = self.width_spin.value()
        target_h = self.height_spin.value()
        keep_ratio = self.radio_keep_ratio.isChecked()

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
        if not backup_dir:
            QMessageBox.warning(self, "错误", "请选择或等待自动生成备份目录。")
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

        self.log_area.clear()
        self.log(f"=== DesktopPet Action Asset Resizer ===")
        self.log(f"pet.json: {pet_path}")
        self.log(f"目标尺寸: {target_w} x {target_h}")
        self.log(f"动作目录: {action_dir}")
        self.log(f"ActionId: {action_id}")
        self.log(f"备份目录: {backup_dir}")
        self.log(f"缩放方式: {'保持比例+居中填充' if keep_ratio else '直接拉伸'}")
        self.log(f"待处理图片: {len(image_files)} 个")

        if gif_files:
            self.log(f"")
            self.log(f"[跳过] 检测到 {len(gif_files)} 个 GIF 文件:")
            for gf in gif_files:
                self.log(f"  - {os.path.basename(gf)}")
            self.log(f"  当前版本暂不处理 GIF，请先拆帧或等待后续版本支持。")

        if not image_files:
            self.log(f"")
            self.log(f"[中止] 没有可处理的图片（仅有 GIF）。")
            return

        # 备份
        self.log(f"")
        self.log(f"--- 开始备份 ---")
        try:
            if os.path.exists(backup_dir):
                # 避免覆盖已有备份
                backup_dir = backup_dir + "_" + str(int(time.time()))
                self.log(f"  备份目录已存在，改用: {backup_dir}")
            shutil.copytree(action_dir, backup_dir)
            self.log(f"  备份完成: {backup_dir}")
        except Exception as e:
            self.log(f"  [错误] 备份失败: {e}")
            QMessageBox.critical(self, "错误", f"备份失败，中止处理。\n{e}")
            return

        # 缩放
        self.log(f"")
        self.log(f"--- 开始缩放 ---")
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

        self.log(f"")
        self.log(f"=== 处理完成 ===")
        self.log(f"成功: {success_count}, 失败: {fail_count}, "
                 f"跳过(已是目标尺寸): {skipped_size_count}, 跳过GIF: {len(gif_files)}")
        self.log(f"备份位于: {backup_dir}")

    @staticmethod
    def resize_image(fpath: str, target_w: int, target_h: int, keep_ratio: bool) -> str:
        """
        缩放单张图片并覆盖原文件。
        返回 "ok" / "skipped" / "failed:<reason>"
        """
        ext = os.path.splitext(fpath)[1].lower()

        # 使用 with 确保文件句柄释放（Windows 覆盖写入安全）
        with Image.open(fpath) as src:
            img = src.copy()

        # 已是目标尺寸则跳过，避免重复压缩导致画质损失
        if img.width == target_w and img.height == target_h:
            return "skipped"

        # 检测是否含透明通道
        has_alpha = img.mode in ("RGBA", "LA") or (
            img.mode == "P" and "transparency" in img.info
        )

        if keep_ratio:
            # 保持比例，居中填充
            img.thumbnail((target_w, target_h), Image.LANCZOS)
            # 创建目标画布
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
            # 直接拉伸
            result = img.resize((target_w, target_h), Image.LANCZOS)

        # 保存
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
    window = MainWindow()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
