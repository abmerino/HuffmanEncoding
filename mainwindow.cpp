#include "mainwindow.h"
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), codeFrequencies(256, 0)
{
    QWidget *center = new QWidget();
    setCentralWidget(center);

    //set vertical box layout as central widget
    QVBoxLayout *mainLayout = new QVBoxLayout(center);

    //create horizontal box within VBox for the buttons
    myButtonLayout = new QHBoxLayout;
    mainLayout->addLayout(myButtonLayout);

    //create first button: load
    myLoadButton = new QPushButton("Load");
    myButtonLayout->addWidget(myLoadButton);
    connect(myLoadButton, &QPushButton::clicked, this, &MainWindow::loadButtonClicked);

    //create second button: encode
    myEncodeButton = new QPushButton("Encode");
    myButtonLayout->addWidget(myEncodeButton);
    connect(myEncodeButton, &QPushButton::clicked, this, &MainWindow::encodeButtonClicked);

    //create third button: decode
    myDecodeButton = new QPushButton("Decode");
    myButtonLayout->addWidget(myDecodeButton);
    connect(myDecodeButton, &QPushButton::clicked, this, &MainWindow::decodeButtonClicked);

    //create table widget
    myTableWidget = new QTableWidget();
    myTableWidget->setRowCount(256); //0-255 byte values for character code
    myTableWidget->setColumnCount(4);
    QStringList headers = {"Byte", "Character", "Frequency", "Encoding"};
    myTableWidget->setHorizontalHeaderLabels(headers);

    mainLayout->addWidget(myTableWidget);

}

void MainWindow::loadButtonClicked() {

    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "All Files (*.*)");

    if (fileName.isEmpty()) return; //user cancelled

    QFile inFile(fileName);

    if (!inFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "Error", QString("Can't open file \"%1\"").arg(fileName));
        return;
    }


    data = inFile.readAll();

    if(data.isEmpty()) {
        QMessageBox::information(this, "Empty File", "Your file is empty, no point encoding it :/");
        return;
    }

    for (int iPos = 0; iPos<data.length(); ++iPos)
        ++codeFrequencies[(unsigned char)data[iPos]];

    myTableWidget->clearContents();
    for (int i = 0; i<256; i++) {
        //column 0: Byte value
        myTableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i)));

        //column 1: printable char or "N/A"
        QString byteChar = (i >= 32 && i <= 126) ? QString(QChar(i)) : "N/A";
        myTableWidget->setItem(i, 1, new QTableWidgetItem(byteChar));

        //column 2: frequency count
        myTableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(codeFrequencies[i])));
    }
}

void MainWindow::encodeButtonClicked() {

    QVector<QString> charCodeEncodingStrings(256, "");


    QMultiMap<int, QByteArray> toDo;
    for (int code = 0; code < 256; ++code) {
        if (codeFrequencies[code] > 0)
            toDo.insert(codeFrequencies[code], QByteArray(1, code));
    }

    QMap<QByteArray, QPair<QByteArray, QByteArray> > parentChildren;
    while(toDo.size() > 1) {

        int freq0 = toDo.begin().key();
        QByteArray chars0 = toDo.begin().value();
        toDo.erase(toDo.begin());

        int freq1 = toDo.begin().key();
        QByteArray chars1 = toDo.begin().value();
        toDo.erase(toDo.begin());

        int parentFreq = freq0 + freq1;
        QByteArray parentChars = chars0 + chars1;
        toDo.insert(parentFreq, parentChars);

        parentChildren[parentChars] = qMakePair(chars0, chars1);
    }

    QByteArray huffmanRoot = toDo.begin().value();

    for ( int i = 0; i < data.length(); ++i) {
        QByteArray current = toDo.begin().value(); //set to root node
        QString code = "";
        QByteArray target = QByteArray(1, data[i]);

        while (current != target) {
            if (parentChildren[current].first.contains(target)) {
                code += "0";
                current = parentChildren[current].first;
            } else {
                code += "1";
                current = parentChildren[current].second;
            }
        }
        charCodeEncodingStrings[(unsigned char)data[i]] = code;
        current = huffmanRoot; //reset to root node for next loop
    }

    QString encoding = ""; //concatenated string of encodings
    for (int iPos = 0; iPos < data.length(); ++iPos) {
        encoding += charCodeEncodingStrings[(unsigned char)data[iPos]];
    }

    QByteArray byteArray;
    int encodingLength = encoding.length();
    for (int i = 0; i < encodingLength; i += 8) {
        QString byteStr = encoding.mid(i, 8);  // Get 8-bit chunk
        if (byteStr.length() < 8) {
            // Pad the last chunk with 0s if it's not 8 bits long
            byteStr = byteStr.leftJustified(8, '0');
        }
        bool ok;
        char byte = byteStr.toInt(&ok, 2);  // Convert binary string to integer
        if (ok) {
            byteArray.append(byte);  // Append converted byte to the byte array
        }
    }

    QString outName = QFileDialog::getSaveFileName(this, "Save", "", "Huffman (*.huf)");
    if (outName.isEmpty()) return;

    QFile outFile(outName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(this, "Error", QString("Can't write to file \"%1\"").arg(outName));
        return;
    }

    QDataStream out(&outFile);
    out << charCodeEncodingStrings;  // Save the Huffman encoding map
    out.writeRawData(byteArray.data(), byteArray.size());  // Save the encoded binary data

    for (int i = 0; i < 256; ++i) {
        if (!charCodeEncodingStrings[i].isEmpty()) {
            myTableWidget->setItem(i, 3, new QTableWidgetItem(charCodeEncodingStrings[i]));
        } else {
            // if no encoding for the byte, leave the cell empty
            myTableWidget->setItem(i, 3, new QTableWidgetItem("N/A"));
        }
    }
}

void MainWindow::decodeButtonClicked() {

    QString fileName = QFileDialog::getOpenFileName(this, "Open encoded file to decode", "", "Huffman Files (*.huf)");

    if (fileName.isEmpty()) return;

    QFile inFile(fileName);

    if (!inFile.exists()) {
        QMessageBox::information(this, "Error", "The file does not exist");
        return;
    }

    if (!inFile.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "Error", QString("Can't open file \"%1\"").arg(fileName));
        return;
    }

    // Read the encoded data and the Huffman map from the file
    QDataStream in(&inFile);

    QVector<QString> charCodeEncodingStrings(256); // stores encoding for each byte

    in >> charCodeEncodingStrings;

    QByteArray encodedBytes;
    encodedBytes = inFile.readAll(); //read the entire binary data

    QString encodedData;
    for (char byte : encodedBytes) {
        encodedData += QString::number((unsigned char)byte, 2).rightJustified(8, '0'); //convert each byte to an 8-bit binary string
    }

    //build a map for decoding the encoded data
    QMap<QString, unsigned char> decodingMap;
    for (int i = 0; i < 256; ++i) {
        if (!charCodeEncodingStrings[i].isEmpty()) {
            decodingMap[charCodeEncodingStrings[i]] = (unsigned char)i;
        }
    }

    //decode encodedData
    QString decodedString = "";
    QString currentCode = "";
    for (int i = 0; i < encodedData.length(); ++i) {
        currentCode += encodedData[i];
        if (decodingMap.contains(currentCode)) {
            decodedString += QChar(decodingMap[currentCode]);
            currentCode = "";
        }
    }

    // Populate the table widget with decoded byte and corresponding character
    myTableWidget->clearContents();
    for (int i = 0; i < 256; ++i) {
        //Column 0: Byte value
        myTableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i)));

        //Column 1: Printable char or "N/A"
        QString byteChar = (i >= 32 && i <= 126) ? QString(QChar(i)) : "N/A";
        myTableWidget->setItem(i, 1, new QTableWidgetItem(byteChar));

        // Column 3: Huffman Encoding
        if (!charCodeEncodingStrings[i].isEmpty()) {
            myTableWidget->setItem(i, 3, new QTableWidgetItem(charCodeEncodingStrings[i]));
        }
    }

    QString outFileName = QFileDialog::getSaveFileName(this, "Save decoded data", "", "Text Files (*.txt)");
    if (outFileName.isEmpty()) return; //user cancelled

    QFile outFile(outFileName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(this, "Error", QString("Can't write to file \"%1\"").arg(outFileName));
        return;
    }

    QTextStream out(&outFile);
    out << decodedString;
}


MainWindow::~MainWindow() {}
