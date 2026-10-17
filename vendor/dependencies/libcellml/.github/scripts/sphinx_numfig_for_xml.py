import argparse
import os

from lxml import html, etree


def update_xml_figure_numbering(location):
    current_dir = os.path.abspath(os.path.curdir)
    working_dir = os.path.join(current_dir, location)
    os.chdir(working_dir)

    for root, dirs, files in os.walk(".", topdown=False):
        for name in files:
            parts = os.path.splitext(name)
            if len(parts) == 2 and parts[1] == ".html":
                partner_xml_file = os.path.join(root, parts[0] + ".xml")
                if os.path.isfile(partner_xml_file):
                    partner_html_file = os.path.join(root, name)
                    # print(partner_html_file)
                    # print(partner_xml_file)

                    with open(partner_html_file, 'r', encoding='utf-8') as html_file:
                        html_doc = html.fromstring(html_file.read())

                    with open(partner_xml_file, 'rb') as xml_file:
                        xml_doc = etree.fromstring(xml_file.read())

                    html_figures = html_doc.findall(".//figure")
                    xml_figures = xml_doc.findall(".//figure")
                    if len(xml_figures) != len(html_figures):
                        print("Yikes!!!!")
                        break

                    if html_figures:
                        for figure_el in html_figures:
                            caption_number = figure_el.find(".//span[@class='caption-number']")
                            if caption_number is not None:
                                figure_id = figure_el.attrib["id"]
                                # Assuming here that the first entry in the ids list is the actual element id.
                                potential_figure_elements = xml_doc.xpath(f".//figure[starts-with(@ids,'{figure_id}')]")

                                if potential_figure_elements and len(potential_figure_elements) == 1:
                                    xml_figure = potential_figure_elements[0]
                                    image_el = xml_figure.find('image')
                                    insert_index = xml_figure.index(image_el)
                                    caption_number_el = etree.fromstring(
                                        f"<caption_number>{caption_number.text}</caption_number>")
                                    xml_figure.insert(insert_index + 1, caption_number_el)
                                else:
                                    print("Double yikes!!!!!", partner_xml_file)

                        # Write out modified xml document.
                        with open(partner_xml_file, "wb") as f:
                            f.write(etree.tostring(xml_doc, pretty_print=True))

    os.chdir(current_dir)


def _process_arguments():
    parser = argparse.ArgumentParser(description="Apply numfig operation for Sphinx XML output.")
    parser.add_argument("directory",
                        help="Relative directory from current working directory to Sphinx output directory.")

    return parser.parse_args()


def main():
    args = _process_arguments()
    update_xml_figure_numbering(args.directory)


if __name__ == "__main__":
    main()
