
/*
 * doccat: concatenate text documents from Word and Writer
 *
 * usage: doccat -o output.doc input1.doc input2.doc input3.doc
 *
 * Author: Santosh Kumar <mbox.santosh@gmail.com>
*/

#include <stdio.h>
#include <sal/main.h>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>

#include <string.h>
#include <unistd.h>

#include <cppuhelper/bootstrap.hxx>
#include <osl/file.hxx>
#include <osl/process.h>

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/document/XDocumentInsertable.hpp>

using namespace rtl;
using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::document;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::text;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;


using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringToOString;

const char *charStr(const OUString s) {
  static char retval[BUFSIZ];
  OString str;
  s.convertToString(&str, RTL_TEXTENCODING_UTF8, RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR);
  const char *o = str.getStr();
  strcpy(retval, o);
  return retval;
}

const char *dirname(const char *path) {
  static char retval[BUFSIZ];
  static char pwd[BUFSIZ];
  static char newdir[BUFSIZ];
  char *s;
  strcpy(retval, path);
  s = strrchr(retval, '/');
  if (s) {
    char *r1;
    int r2;
    *s = 0;
    r1 = getcwd(pwd, BUFSIZ);
    r2 = chdir(retval);
    r1 = getcwd(newdir, BUFSIZ);
    strcpy(retval, newdir);
    r2 = chdir(pwd);
  }
  return retval;
}

const char *createFileName(const char *filename) {
  static char retval[BUFSIZ*2];
  static char pwd[BUFSIZ];
  static char newdir[BUFSIZ];
  char *r1;
  const char *p;
  r1 = getcwd(pwd, BUFSIZ);
  if (!(p = strrchr(filename, '/'))) sprintf(retval, "%s/%s", pwd, filename);
  else sprintf(retval, "%s/%s", dirname(filename), p+1);
  return retval;
}

 
SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
  static char fileUrl[1024];
  char *outfilename = (char *) charStr("out.doc");
  int ai = 1;
  if (!strcmp(argv[ai], "-o")) {
    outfilename = argv[++ai];
    ++ai;
  }
  setbuf(stdout, 0);

  try {
    // get the remote office component context
    Reference<XComponentContext> xContext(::cppu::bootstrap());
    if (!xContext.is()) {
      fprintf(stdout, "Error getting context from running LO instance...\n");
      return -1;
    }
  
    // retrieve the service-manager from the context
    Reference<XMultiComponentFactory> xServiceManager = xContext->getServiceManager();
    if (xServiceManager.is())
      fprintf(stdout, "remote ServiceManager is available\n");
    else {
      fprintf(stdout, "remote ServiceManager is not available\n");
      return -1;
    }

    // Create Desktop object, xDesktop is still a XInterface, need to downcast it.
    Reference<XInterface> xDesktop = xServiceManager->createInstanceWithContext(OUString("com.sun.star.frame.Desktop"), xContext);

    if (!xDesktop.is()) {
      fprintf(stdout, "Error creating com.sun.star.frame.Desktop object");
      fflush(stdout);
      return -1;
    }

    // Get XDesktop2 from XInterface type (downcast) indirectly using XInterface's queryInterface() method.
    Reference<XDesktop2> xDesktop2(xDesktop, UNO_QUERY_THROW);

    if (!xDesktop2.is()) {
      fprintf(stdout, "Error upcasting XInterface to XDesktop2");
      fflush(stdout);
      return -1;
    }

    // open a text document
    Reference<XComponent> xComponent = xDesktop2->loadComponentFromURL(
      "private:factory/swriter",
      OUString::createFromAscii("_blank"), 0,
      Sequence <::com::sun::star::beans::PropertyValue>());
    if (!xComponent.is()) {
      fprintf(stdout, "error opening text document\n");
      return -1;
    }


    Reference<XTextDocument> xTextDocument(xComponent, UNO_QUERY_THROW);

    Reference<XComponent> xCurrentComponent = xDesktop2->getCurrentComponent();

    // get the XModel interface from the component
    Reference<XModel> xModel(xCurrentComponent, UNO_QUERY_THROW);

    // the model knows its controller
    Reference<XController> xController = xModel->getCurrentController();

    // the controller gives us the TextViewCursor
    // query the viewcursor supplier interface
    Reference<XTextViewCursorSupplier> xViewCursorSupplier(xController, UNO_QUERY_THROW);

    // get the cursor
    Reference<XTextViewCursor> xViewCursor = xViewCursorSupplier->getViewCursor();

    Reference<XText> xDocumentText = xViewCursor->getText();

    Reference<XTextCursor> xModelCursor = xDocumentText->createTextCursorByRange(xViewCursor->getStart());

    Reference<XDocumentInsertable> doc(xModelCursor, UNO_QUERY_THROW);

    for (; argv[ai]; ai++) {
        static char fileUrl[1024];
        const char *fileName = argv[ai];
        sprintf(fileUrl, "file://%s", createFileName(fileName));
        const char *fn = fileUrl;
        fprintf(stdout, "adding document - %s\n", fn);

        xModelCursor->gotoEnd(false);
        doc->insertDocumentFromURL(OUString::createFromAscii(fn), Sequence <::com::sun::star::beans::PropertyValue>());
    }

    Reference<XStorable> xStorable(xTextDocument, UNO_QUERY_THROW);
    sprintf(fileUrl, "file://%s", createFileName(outfilename));
    xStorable->storeToURL(OUString::createFromAscii(fileUrl), Sequence <::com::sun::star::beans::PropertyValue>());
    fprintf(stdout, "saved text document - %s\n", fileUrl);
  
  }
  catch (::cppu::BootstrapException& e) {
    fprintf(stderr, "caught BootstrapException: %s\n",
      OUStringToOString(e.getMessage(), RTL_TEXTENCODING_ASCII_US).getStr());
    fflush(stderr);
    return -1;
  }
  catch (Exception& e) {
    fprintf(stderr, "caught UNO exception: %s\n",
        OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
    fflush(stderr);
    return -1;
  }

  return 0;
}

