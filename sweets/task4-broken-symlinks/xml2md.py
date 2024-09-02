import xml.etree.ElementTree as ET


def xml2md(elem: ET.Element, idx=0):
    if elem.tag == 'p':
        return ((elem.text if elem.text else '') +
                ''.join(xml2md(inner) for inner in elem) +
                (elem.tail if elem.tail else '') + '\n')
    if elem.tag == 'ul':
        return ((elem.text if elem.text else '') + '\n' +
                ''.join(xml2md(inner, i) for i, inner in enumerate(elem)) +
                (elem.tail if elem.tail else '') + '\n')
    if elem.tag == 'li':
        return (f'{idx + 1}) ' + (elem.text if elem.text else '') +
                ''.join(xml2md(inner, i) for i, inner in enumerate(elem)) +
                (elem.tail if elem.tail else '') + '\n')
    if elem.tag == 'pre':
        return '```\n' + ''.join(elem.itertext()) + '\n```' + (elem.tail if elem.tail else '')
    if elem.tag == 'b':
        return '**' + ''.join(elem.itertext()) + '**' + (elem.tail if elem.tail else '')
    if elem.tag == 'tt' or elem.tag == 'code':
        return '`' + ''.join(elem.itertext()) + '`' + (elem.tail if elem.tail else '')
    raise RuntimeError(f"Unsupported {elem}")


tree = ET.parse('statement.xml')
root = tree.getroot()
with open('README.md', 'w') as f:
    for elem in root.find('statement').find('description'):
        f.write(xml2md(elem))
        f.write('\n')
