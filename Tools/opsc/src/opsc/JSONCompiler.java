/*
 * JSONCompiler.java
 *
 * Created on den 24 september 2016
 *
 */

package opsc;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Vector;
import parsing.AbstractTemplateBasedIDLCompiler;
import parsing.IDLClass;
import parsing.IDLField;
import parsing.TopicInfo;

/**
 *
 * @author Lelle
 */
public class JSONCompiler extends CompilerSupport
{
    String res = "";

    public JSONCompiler(String projname) {
        super(projname);
        //tab is 2 spaces
        setTabString("  ");
    }

    public void compileDataClasses(Vector<IDLClass> idlClasses, String projectDirectory)
    {
        res = "";
        this._idlClasses = idlClasses;

        res += "[" + endl();
        res += generate_coretypes_JSON(1);
        res += tab(1) + "," + endl();
        res += generate_OPSObject_JSON(1);
        for (IDLClass iDLClass : idlClasses)
        {
            res += tab(1) + "," + endl();
            res += generateJSONobject(1, iDLClass);
        }
        res += "]" + endl();

        //Save the generated text to the output file.
        setOutputFileName(_outputDir + File.separator + _projectName + ".json");
        saveOutputText(res);
        System.out.println("Info: Saved JSON description to file: " + outputFileName);
    }

    public void compileTopicConfig(Vector<TopicInfo> topics, String name, String packageString, String projectDirectory)
    {}

    protected String generate_coretypes_JSON(int t)
    {
        String res = "";
        res += tab(t) + "{" + endl();
        res += tab(t+1) + "\"coretypes\": [" + endl();
        res += tab(t+2) + "{ \"type\": \"boolean\", \"size\": 1 }," + endl();
        res += tab(t+2) + "{ \"type\": \"byte\", \"size\": 1 }," + endl();
        res += tab(t+2) + "{ \"type\": \"short\", \"size\": 2 }," + endl();
        res += tab(t+2) + "{ \"type\": \"int\", \"size\": 4 }," + endl();
        res += tab(t+2) + "{ \"type\": \"long\", \"size\": 8 }," + endl();
        res += tab(t+2) + "{ \"type\": \"float\", \"size\": 4 }," + endl();
        res += tab(t+2) + "{ \"type\": \"double\", \"size\": 8 }," + endl();

        res += tab(t+2) + "{ \"type\": \"string\"," + endl();
        res += tab(t+3) + "\"comp\": [" + endl();
        res += tab(t+4) + "{ \"num_elements\": \"int\" }," + endl();
        res += tab(t+4) + "{ \"elements\": \"byte\" }" + endl();
        res += tab(t+3) + "]" + endl();
        res += tab(t+2) + "}," + endl();

        res += tab(t+2) + "{ \"type\": \"vector<T>\"," + endl();
        res += tab(t+3) + "\"comp\": [" + endl();
        res += tab(t+4) + "{ \"num_elements\": \"int\" }," + endl();
        res += tab(t+4) + "{ \"elements\": \"T\" }" + endl();
        res += tab(t+3) + "]" + endl();
        res += tab(t+2) + "}" + endl();

        res += tab(t+1) + "]," + endl();
        res += tab(t+1) + "\"alignment\": 1," + endl();
        res += tab(t+1) + "\"endianess\": \"little-endian\"" + endl();
        res += tab(t) + "}" + endl();
        return res;
    }

    protected String generate_OPSObject_JSON(int t)
    {
        String res = "";
        res += tab(t) + "{" + endl();
        res += tab(t+1) + "\"type\": \"ops.OPSObject\"," + endl();

        res += tab(t+1) + "\"fields\": [" + endl();
        res += tab(t+2) + "{";
        res += " \"name\": \"key\",";
        res += " \"type\": \"string\"";
        res += " }" + endl();
        res += tab(t+1) + "]" + endl();

        res += tab(t) + "}" + endl();
        return res;
    }

    protected String makeType(IDLField field)
    {
      String ty = field.getType().replace("[]", "");
      if (field.isIdlType()) {
        ty = getFullyQualifiedClassName(ty);
      }
      if (field.isAbstract()) {
        return "virtual " + ty;
      } else {
        return ty;
      }
    }

    protected String generateJSONobject(int t, IDLClass idlClass)
    {
        String res = "";
        res += tab(t) + "{" + endl();
        res += tab(t+1) + "\"type\": \"" + idlClass.getPackageName() + "." + idlClass.getClassName() + "\"," + endl();
        String baseClass = "ops.OPSObject";
        if (idlClass.getBaseClassName() != null) {
          baseClass = idlClass.getBaseClassName();
        }
        res += tab(t+1) + "\"extends\": \"" + getFullyQualifiedClassName(baseClass) + "\"," + endl();

        if (idlClass.getType() == IDLClass.ENUM_TYPE) {
          res += tab(t+1) + "\"enum\": [";
          String pre = "";
          for (String str : idlClass.getEnumNames()) {
            res += pre + "\"" + str + "\"";
            pre = ", ";
          }
          res += "]" + endl();

        } else {
          res += tab(t+1) + "\"fields\": [" + endl();

          boolean first = true;
          for (IDLField field : idlClass.getFields()) {
            if (!first) res += "," + endl();
            first = false;
            res += tab(t+2) + "{";
            res += " \"name\": \"" + field.getName() + "\"";
            if (field.isArray()) {
              res += ", \"type\": \"vector<T>\", \"elementtype\": ";

            } else {
              res += ", \"type\": ";

            }
            res += "\"" + makeType(field) + "\"";

            String comment = field.getComment();
            comment = comment.replace("/*", "").replace("*/", "").replace("\n", " ").trim();
            if (comment.length() > 0) {
              res += ", \"desc\": \"" + comment + "\"";
            }
            res += " }";
          }

          res += endl() + tab(t+1) + "]" + endl();
        }
        res += tab(t) + "}" + endl();
        return res;
    }

    protected String getLastPart(String name)
    {
      int idx;
      String s = name;
      while ((idx = s.indexOf('.')) > 0) {
        s = s.substring(idx+1);
      }
      return s;
    }

    protected String getFullyQualifiedClassName(String className)
    {
      String s = getLastPart(className);
      for (IDLClass cl : this._idlClasses) {
        if (cl.getClassName().equals(s)) {
          return cl.getPackageName() + "." + cl.getClassName();
        }
      }
      // Didn't find class in list so it is from another "project"
      // Assume then that the 'className' contains <packagename>.<class>
      // and that the unit is <packagename>.<class>, i.e. the same
      return className;
    }

    public String getName()
    {
        return "JSONIDLCompiler";
    }

}
