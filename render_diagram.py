#!/usr/bin/env python3
"""
Generate a monochrome UML activity diagram as SVG.

The output is intentionally plain and printer-friendly so it can be
used in coursework reports that require a strict black-and-white style.
"""

from __future__ import annotations

import html
from pathlib import Path


FONT_FAMILY = "Arial, Helvetica, sans-serif"
FONT_SIZE = 14
STROKE = "#000000"
FILL = "#ffffff"
PAGE_WIDTH = 1200
MARGIN_X = 80
CURRENT_Y = 40
SVG_ELEMENTS: list[str] = []


def svg_text(x: float, y: float, text: str, anchor: str = "middle", weight: str = "normal") -> None:
    SVG_ELEMENTS.append(
        f'<text x="{x}" y="{y}" text-anchor="{anchor}" '
        f'font-family="{FONT_FAMILY}" font-size="{FONT_SIZE}" '
        f'font-weight="{weight}" fill="{STROKE}">{html.escape(text)}</text>'
    )


def draw_multiline_text(x: float, y: float, lines: list[str], line_height: int = 18) -> None:
    start_y = y - ((len(lines) - 1) * line_height) / 2
    for index, line in enumerate(lines):
        svg_text(x, start_y + index * line_height, line)


def add_arrowhead() -> None:
    SVG_ELEMENTS.append(
        """
<defs>
  <marker id="arrow" markerWidth="10" markerHeight="8" refX="9" refY="4" orient="auto" markerUnits="strokeWidth">
    <path d="M 0 0 L 10 4 L 0 8 z" fill="#000000" />
  </marker>
</defs>
""".strip()
    )


def draw_terminal(label: str) -> tuple[float, float]:
    global CURRENT_Y
    center_x = PAGE_WIDTH / 2
    radius = 18

    if label == "start":
        SVG_ELEMENTS.append(
            f'<circle cx="{center_x}" cy="{CURRENT_Y + radius}" r="{radius}" '
            f'stroke="{STROKE}" fill="{STROKE}" stroke-width="2" />'
        )
    else:
        SVG_ELEMENTS.append(
            f'<circle cx="{center_x}" cy="{CURRENT_Y + radius}" r="{radius}" '
            f'stroke="{STROKE}" fill="{FILL}" stroke-width="2" />'
        )
        SVG_ELEMENTS.append(
            f'<circle cx="{center_x}" cy="{CURRENT_Y + radius}" r="{radius - 6}" '
            f'stroke="{STROKE}" fill="{STROKE}" stroke-width="2" />'
        )

    top = CURRENT_Y
    bottom = CURRENT_Y + radius * 2
    CURRENT_Y = bottom + 30
    return top, bottom


def draw_process(lines: list[str], width: int = 420, height: int | None = None) -> tuple[float, float]:
    global CURRENT_Y
    center_x = PAGE_WIDTH / 2
    if height is None:
        height = max(56, 26 + len(lines) * 18)
    x = center_x - width / 2
    y = CURRENT_Y
    SVG_ELEMENTS.append(
        f'<rect x="{x}" y="{y}" width="{width}" height="{height}" rx="18" ry="18" '
        f'stroke="{STROKE}" fill="{FILL}" stroke-width="2" />'
    )
    draw_multiline_text(center_x, y + height / 2 + 5, lines)
    top = y
    bottom = y + height
    CURRENT_Y = bottom + 30
    return top, bottom


def draw_note(lines: list[str], x: float, y: float, width: int = 260) -> None:
    height = 24 + len(lines) * 18
    SVG_ELEMENTS.append(
        f'<rect x="{x}" y="{y}" width="{width}" height="{height}" '
        f'stroke="{STROKE}" fill="{FILL}" stroke-width="1.5" stroke-dasharray="6,4" />'
    )
    draw_multiline_text(x + width / 2, y + height / 2 + 5, lines)


def draw_decision(lines: list[str], width: int = 260, height: int = 110) -> tuple[float, float]:
    global CURRENT_Y
    center_x = PAGE_WIDTH / 2
    center_y = CURRENT_Y + height / 2
    left = center_x - width / 2
    right = center_x + width / 2
    top = CURRENT_Y
    bottom = CURRENT_Y + height
    points = f"{center_x},{top} {right},{center_y} {center_x},{bottom} {left},{center_y}"
    SVG_ELEMENTS.append(
        f'<polygon points="{points}" stroke="{STROKE}" fill="{FILL}" stroke-width="2" />'
    )
    draw_multiline_text(center_x, center_y + 5, lines)
    CURRENT_Y = bottom + 30
    return top, bottom


def draw_arrow(x1: float, y1: float, x2: float, y2: float, label: str | None = None,
               label_x: float | None = None, label_y: float | None = None) -> None:
    SVG_ELEMENTS.append(
        f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{STROKE}" stroke-width="2" marker-end="url(#arrow)" />'
    )
    if label:
        if label_x is None:
            label_x = (x1 + x2) / 2
        if label_y is None:
            label_y = (y1 + y2) / 2 - 6
        svg_text(label_x, label_y, label)


def render() -> str:
    global CURRENT_Y, SVG_ELEMENTS

    CURRENT_Y = 40
    SVG_ELEMENTS = []
    add_arrowhead()

    svg_text(PAGE_WIDTH / 2, 28, "Диаграмма деятельности клиентского приложения", weight="bold")
    svg_text(PAGE_WIDTH / 2, 48, "(оформление в черно-белом стиле для отчета)", anchor="middle")

    start_top, start_bottom = draw_terminal("start")
    p1_top, p1_bottom = draw_process(["Проверить наличие", "аргументов командной строки"])
    d1_top, d1_bottom = draw_decision(["Аргументы", "переданы?"])
    p_help_top, p_help_bottom = draw_process(["Вывести справку", "по использованию программы"], width=360)
    end_help_top, end_help_bottom = draw_terminal("end")

    branch_right_x = 940
    branch_join_y = p_help_bottom + 30

    p2_top, p2_bottom = draw_process(["Создать объект Client"])
    p3_top, p3_bottom = draw_process(["Запустить метод", "Client.run(argc, argv)"])
    p4_top, p4_bottom = draw_process(["Разобрать аргументы", "командной строки"])
    d2_top, d2_bottom = draw_decision(["Аргументы", "корректны?"])

    p_cfg_top, p_cfg_bottom = draw_process(["Прочитать файл конфигурации"])
    draw_note(
        ["Из файла считываются:", "логин и пароль."],
        x=880,
        y=p_cfg_top + 6,
        width=220,
    )
    d3_top, d3_bottom = draw_decision(["Конфигурация", "прочитана?"])
    p_vec_top, p_vec_bottom = draw_process(["Считать векторы", "из бинарного файла"])
    d4_top, d4_bottom = draw_decision(["Входной файл", "обработан", "успешно?"])
    p_val_top, p_val_bottom = draw_process(["Проверить корректность", "считанных данных"])
    d5_top, d5_bottom = draw_decision(["Данные", "валидны?"])
    p_conn_top, p_conn_bottom = draw_process(["Установить TCP-соединение", "с сервером"])
    d6_top, d6_bottom = draw_decision(["Соединение", "установлено?"])
    p_auth1_top, p_auth1_bottom = draw_process(["Отправить серверу логин"])
    p_auth2_top, p_auth2_bottom = draw_process(["Получить соль SALT16", "или сообщение ERR"])
    d7_top, d7_bottom = draw_decision(["Получен ERR", "или неверная соль?"])
    p_auth3_top, p_auth3_bottom = draw_process(["Вычислить HASH =", "MD5(SALT + PASSWORD)"])
    p_auth4_top, p_auth4_bottom = draw_process(["Отправить HASH серверу"])
    p_auth5_top, p_auth5_bottom = draw_process(["Получить ответ", "аутентификации"])
    d8_top, d8_bottom = draw_decision(["Ответ сервера", "равен OK?"])
    p_send_count_top, p_send_count_bottom = draw_process(["Отправить количество векторов"])
    p_loop_select_top, p_loop_select_bottom = draw_process(["Выбрать очередной вектор"])
    p_loop_size_top, p_loop_size_bottom = draw_process(["Отправить размер вектора"])
    d9_top, d9_bottom = draw_decision(["Размер", "вектора > 0?"])
    p_loop_values_top, p_loop_values_bottom = draw_process(["Отправить элементы вектора"])
    p_loop_result_top, p_loop_result_bottom = draw_process(["Получить результат", "обработки вектора"])
    d10_top, d10_bottom = draw_decision(["Результат", "получен?"])
    p_loop_store_top, p_loop_store_bottom = draw_process(["Сохранить результат", "во внутренний список"])
    d11_top, d11_bottom = draw_decision(["Остались", "неотправленные", "векторы?"])
    p_save_top, p_save_bottom = draw_process(["Сохранить результаты", "в выходной файл"])
    d12_top, d12_bottom = draw_decision(["Сохранение", "выполнено?"])
    p_close_ok_top, p_close_ok_bottom = draw_process(["Закрыть соединение"])
    p_success_top, p_success_bottom = draw_process(["Завершить работу программы", "с кодом успеха"], width=430)
    end_ok_top, end_ok_bottom = draw_terminal("end")

    error_x = 205
    error_width = 290
    error_boxes = [
        ("Вывести сообщение об ошибке\nи завершить работу", d2_top + 5),
        ("Ошибка чтения конфигурации", d3_top + 5),
        ("Ошибка чтения входных данных", d4_top + 15),
        ("Ошибка валидации данных", d5_top + 15),
        ("Ошибка подключения к серверу", d6_top + 15),
        ("Ошибка аутентификации", d7_top + 15),
        ("Ошибка аутентификации", d8_top + 15),
        ("Ошибка обмена данными", d10_top + 10),
        ("Ошибка сохранения результатов", d12_top + 15),
    ]
    error_rects: list[tuple[float, float, float, float]] = []
    for text, y in error_boxes:
        height = 60
        x = error_x
        SVG_ELEMENTS.append(
            f'<rect x="{x}" y="{y}" width="{error_width}" height="{height}" rx="18" ry="18" '
            f'stroke="{STROKE}" fill="{FILL}" stroke-width="2" />'
        )
        draw_multiline_text(x + error_width / 2, y + height / 2 + 5, text.split("\n"))
        error_rects.append((x, y, x + error_width, y + height))

    close_error_x = error_x + error_width / 2
    close_error_y = d7_bottom + 35
    SVG_ELEMENTS.append(
        f'<rect x="{error_x}" y="{close_error_y}" width="{error_width}" height="60" rx="18" ry="18" '
        f'stroke="{STROKE}" fill="{FILL}" stroke-width="2" />'
    )
    draw_multiline_text(close_error_x, close_error_y + 35, ["Закрыть соединение"])
    error_rects.insert(6, (error_x, close_error_y, error_x + error_width, close_error_y + 60))

    final_error_y = d12_bottom + 105
    final_end_center_y = final_error_y + 105
    SVG_ELEMENTS.append(
        f'<circle cx="{close_error_x}" cy="{final_end_center_y}" r="18" stroke="{STROKE}" fill="{FILL}" stroke-width="2" />'
    )
    SVG_ELEMENTS.append(
        f'<circle cx="{close_error_x}" cy="{final_end_center_y}" r="12" stroke="{STROKE}" fill="{STROKE}" stroke-width="2" />'
    )

    # Main vertical flow
    center_x = PAGE_WIDTH / 2
    flow_pairs = [
        (start_bottom, p1_top),
        (p1_bottom, d1_top),
        (p2_bottom, p3_top),
        (p3_bottom, p4_top),
        (p4_bottom, d2_top),
        (p_cfg_bottom, d3_top),
        (p_vec_bottom, d4_top),
        (p_val_bottom, d5_top),
        (p_conn_bottom, d6_top),
        (p_auth1_bottom, p_auth2_top),
        (p_auth3_bottom, p_auth4_top),
        (p_auth4_bottom, p_auth5_top),
        (p_send_count_bottom, d11_top),
        (p_save_bottom, d12_top),
        (p_close_ok_bottom, p_success_top),
        (p_success_bottom, end_ok_top),
    ]
    for y1, y2 in flow_pairs:
        draw_arrow(center_x, y1, center_x, y2)

    # First decision
    d1_center_y = (d1_top + d1_bottom) / 2
    draw_arrow(center_x + 130, d1_center_y, branch_right_x, d1_center_y, label="нет", label_y=d1_center_y - 10)
    draw_arrow(branch_right_x, d1_center_y, branch_right_x, p2_top - 10)
    draw_arrow(branch_right_x, p2_top - 10, center_x, p2_top - 10)
    draw_arrow(center_x, p2_top - 10, center_x, p2_top)
    draw_arrow(center_x, d1_bottom, center_x, p_help_top, label="да", label_x=center_x + 38, label_y=(d1_bottom + p_help_top) / 2)
    draw_arrow(center_x, p_help_bottom, center_x, end_help_top)

    # Parse args decision to error
    d2_center_y = (d2_top + d2_bottom) / 2
    rect = error_rects[0]
    draw_arrow(center_x - 130, d2_center_y, rect[2], d2_center_y, label="нет", label_y=d2_center_y - 10)
    draw_arrow(rect[2], d2_center_y, rect[0] + error_width / 2, rect[1])
    draw_arrow(center_x, d2_bottom, center_x, p_cfg_top, label="да", label_x=center_x + 38, label_y=(d2_bottom + p_cfg_top) / 2)

    # Config read decision
    d3_center_y = (d3_top + d3_bottom) / 2
    rect = error_rects[1]
    draw_arrow(center_x - 130, d3_center_y, rect[2], d3_center_y, label="нет", label_y=d3_center_y - 10)
    draw_arrow(rect[2], d3_center_y, rect[0] + error_width / 2, rect[1])
    draw_arrow(center_x, d3_bottom, center_x, p_vec_top, label="да", label_x=center_x + 38, label_y=(d3_bottom + p_vec_top) / 2)

    # File read decision
    d4_center_y = (d4_top + d4_bottom) / 2
    rect = error_rects[2]
    draw_arrow(center_x - 130, d4_center_y, rect[2], d4_center_y, label="нет", label_y=d4_center_y - 10)
    draw_arrow(rect[2], d4_center_y, rect[0] + error_width / 2, rect[1])
    draw_arrow(center_x, d4_bottom, center_x, p_val_top, label="да", label_x=center_x + 38, label_y=(d4_bottom + p_val_top) / 2)

    # Validation decision
    d5_center_y = (d5_top + d5_bottom) / 2
    rect = error_rects[3]
    draw_arrow(center_x - 130, d5_center_y, rect[2], d5_center_y, label="нет", label_y=d5_center_y - 10)
    draw_arrow(rect[2], d5_center_y, rect[0] + error_width / 2, rect[1])
    draw_arrow(center_x, d5_bottom, center_x, p_conn_top, label="да", label_x=center_x + 38, label_y=(d5_bottom + p_conn_top) / 2)

    # Connection decision
    d6_center_y = (d6_top + d6_bottom) / 2
    rect = error_rects[4]
    draw_arrow(center_x - 130, d6_center_y, rect[2], d6_center_y, label="нет", label_y=d6_center_y - 10)
    draw_arrow(rect[2], d6_center_y, rect[0] + error_width / 2, rect[1])
    draw_arrow(center_x, d6_bottom, center_x, p_auth1_top, label="да", label_x=center_x + 38, label_y=(d6_bottom + p_auth1_top) / 2)

    # auth step 1->2 and decision
    draw_arrow(center_x, p_auth2_bottom, center_x, d7_top)

    d7_center_y = (d7_top + d7_bottom) / 2
    rect = error_rects[5]
    draw_arrow(center_x - 130, d7_center_y, rect[2], d7_center_y, label="да", label_y=d7_center_y - 10)
    draw_arrow(rect[2], d7_center_y, rect[0] + error_width / 2, rect[1])
    close_rect = error_rects[6]
    draw_arrow(rect[0] + error_width / 2, rect[3], rect[0] + error_width / 2, close_rect[1])
    draw_arrow(center_x, d7_bottom, center_x, p_auth3_top, label="нет", label_x=center_x + 45, label_y=(d7_bottom + p_auth3_top) / 2)

    draw_arrow(center_x, p_auth5_bottom, center_x, d8_top)
    d8_center_y = (d8_top + d8_bottom) / 2
    rect = error_rects[7]
    draw_arrow(center_x - 130, d8_center_y, rect[2], d8_center_y, label="нет", label_y=d8_center_y - 10)
    draw_arrow(rect[2], d8_center_y, rect[0] + error_width / 2, rect[1])
    draw_arrow(rect[0] + error_width / 2, rect[3], rect[0] + error_width / 2, close_rect[1])
    draw_arrow(center_x, d8_bottom, center_x, p_send_count_top, label="да", label_x=center_x + 38, label_y=(d8_bottom + p_send_count_top) / 2)

    # Loop control
    draw_arrow(center_x, d11_bottom, center_x, p_loop_select_top, label="да", label_x=center_x + 38, label_y=(d11_bottom + p_loop_select_top) / 2)
    draw_arrow(center_x, p_loop_select_bottom, center_x, p_loop_size_top)
    draw_arrow(center_x, p_loop_size_bottom, center_x, d9_top)

    d9_center_y = (d9_top + d9_bottom) / 2
    draw_arrow(center_x, d9_bottom, center_x, p_loop_result_top, label="нет", label_x=center_x + 45, label_y=(d9_bottom + p_loop_result_top) / 2)
    draw_arrow(center_x + 130, d9_center_y, 855, d9_center_y, label="да", label_y=d9_center_y - 10)
    draw_arrow(855, d9_center_y, 855, p_loop_values_top - 10)
    draw_arrow(855, p_loop_values_top - 10, center_x, p_loop_values_top - 10)
    draw_arrow(center_x, p_loop_values_top - 10, center_x, p_loop_values_top)
    draw_arrow(center_x, p_loop_values_bottom, center_x, p_loop_result_top)

    draw_arrow(center_x, p_loop_result_bottom, center_x, d10_top)
    d10_center_y = (d10_top + d10_bottom) / 2
    rect = error_rects[8]
    draw_arrow(center_x - 130, d10_center_y, rect[2], d10_center_y, label="нет", label_y=d10_center_y - 10)
    draw_arrow(rect[2], d10_center_y, close_rect[0] + error_width / 2, d10_center_y)
    draw_arrow(close_rect[0] + error_width / 2, d10_center_y, close_rect[0] + error_width / 2, close_rect[1])
    draw_arrow(center_x, d10_bottom, center_x, p_loop_store_top, label="да", label_x=center_x + 38, label_y=(d10_bottom + p_loop_store_top) / 2)
    draw_arrow(center_x, p_loop_store_bottom, center_x, d11_top)

    # Exit loop
    d11_center_y = (d11_top + d11_bottom) / 2
    draw_arrow(center_x + 130, d11_center_y, 915, d11_center_y, label="нет", label_y=d11_center_y - 10)
    draw_arrow(915, d11_center_y, 915, p_save_top - 10)
    draw_arrow(915, p_save_top - 10, center_x, p_save_top - 10)
    draw_arrow(center_x, p_save_top - 10, center_x, p_save_top)

    d12_center_y = (d12_top + d12_bottom) / 2
    rect = error_rects[9]
    draw_arrow(center_x - 130, d12_center_y, rect[2], d12_center_y, label="нет", label_y=d12_center_y - 10)
    draw_arrow(rect[2], d12_center_y, close_rect[0] + error_width / 2, d12_center_y)
    draw_arrow(close_rect[0] + error_width / 2, d12_center_y, close_rect[0] + error_width / 2, close_rect[1])
    draw_arrow(center_x, d12_bottom, center_x, p_close_ok_top, label="да", label_x=center_x + 38, label_y=(d12_bottom + p_close_ok_top) / 2)

    # Common error close to final end
    draw_arrow(close_rect[0] + error_width / 2, close_rect[3], close_rect[0] + error_width / 2, final_end_center_y - 18)

    height = int(max(end_ok_bottom + 40, final_end_center_y + 40))
    content = "\n".join(SVG_ELEMENTS)
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{PAGE_WIDTH}" height="{height}" '
        f'viewBox="0 0 {PAGE_WIDTH} {height}">\n'
        f'<rect width="100%" height="100%" fill="{FILL}" />\n'
        f"{content}\n"
        f"</svg>\n"
    )


def main() -> int:
    output_path = Path("activity_diagram_gost.svg")
    output_path.write_text(render(), encoding="utf-8")
    print(f"Saved SVG to {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
