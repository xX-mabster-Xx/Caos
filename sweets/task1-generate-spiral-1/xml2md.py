import xml.etree.ElementTree as ET


def xml2md(elem: ET.Element):
    if elem.tag == 'p':
        return elem.text + ''.join(xml2md(inner) for inner in elem) + elem.tail + '\n'
    if elem.tag == 'pre':
        return '```\n' + ''.join(elem.itertext()) + '\n```' + elem.tail
    if elem.tag == 'b':
        return '**' + ''.join(elem.itertext()) + '**' + elem.tail
    raise RuntimeError(f"Unsupported {elem}")


tree = ET.parse('statement.xml')
root = tree.getroot()
with open('README.md', 'w') as f:
    for elem in root.find('statement').find('description'):
        f.write(xml2md(elem))
        f.write('\n')
