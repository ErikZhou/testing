
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace XmlTool
{
    public static class XmlTool
    {

        public static XmlDocument ParseFile(string filePath)
        {
            XmlDocument doc = new XmlDocument();
            doc.Load(filePath);

            //foreach (XmlNode node in doc.DocumentElement.ChildNodes)
            //{
            //    string text = node.InnerText; //or loop through its children as well
            //    str += text;
            //    //  Console.WriteLine(node.InnerText + "\n");
            //}
            return doc;
        }

        public static string GetAttribute(XmlDocument doc, string name)
        {
            return doc.DocumentElement.SelectSingleNode(name).InnerText;
        }
        public static void SetAttributeAndSave(string filePath, XmlDocument doc, string name, string value)
        {
            doc.DocumentElement.SelectSingleNode(name).InnerText = value;
            doc.Save(filePath);
        }

        public static void SetAttribute(XmlDocument doc, string name, string value)
        {
            doc.DocumentElement.SelectSingleNode(name).InnerText = value;
        }
        public static void SaveXml(string filePath, XmlDocument doc)
        {
            doc.Save(filePath);
        }

    }
}
