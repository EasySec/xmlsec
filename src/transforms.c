/** 
 * XML Security Library
 *
 * The Transforms Element (http://www.w3.org/TR/xmldsig-core/#sec-Transforms)
 * 
 * The optional Transforms element contains an ordered list of Transform 
 * elements; these describe how the signer obtained the data object that 
 * was digested.
 *
 * Schema Definition:
 * 
 *  <element name="Transforms" type="ds:TransformsType"/>
 *  <complexType name="TransformsType">
 *    <sequence>
 *      <element ref="ds:Transform" maxOccurs="unbounded"/> 
 *    </sequence>
 *   </complexType>
 *
 *  <element name="Transform" type="ds:TransformType"/>
 *  <complexType name="TransformType" mixed="true">
 *    <choice minOccurs="0" maxOccurs="unbounded"> 
 *      <any namespace="##other" processContents="lax"/>
 *      <!-- (1,1) elements from (0,unbounded) namespaces -->
 *      <element name="XPath" type="string"/> 
 *    </choice>
 *    <attribute name="Algorithm" type="anyURI" use="required"/> 
 *  </complexType>
 *    
 * DTD:
 *    
 *  <!ELEMENT Transforms (Transform+)>
 *  <!ELEMENT Transform (#PCDATA|XPath %Transform.ANY;)* >
 *  <!ATTLIST Transform Algorithm    CDATA    #REQUIRED >
 *  <!ELEMENT XPath (#PCDATA) >
 * 
 * See Copyright for the status of this software.
 * 
 * Author: Aleksey Sanin <aleksey@aleksey.com>
 */

#include "globals.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpointer.h>

#include <xmlsec/xmlsec.h>
#include <xmlsec/buffer.h>
#include <xmlsec/xmltree.h>
#include <xmlsec/keysdata.h>
#include <xmlsec/keys.h>
#include <xmlsec/keyinfo.h>
#include <xmlsec/transforms.h>
#include <xmlsec/base64.h>
#include <xmlsec/io.h>
#include <xmlsec/membuf.h>
#include <xmlsec/parser.h>
#include <xmlsec/errors.h>

/**************************************************************************
 *
 * Global xmlSecTransformIds list functions
 *
 *************************************************************************/
static xmlSecPtrList xmlSecAllTransformIds;

xmlSecPtrListPtr
xmlSecTransformIdsGet(void) {
    return(&xmlSecAllTransformIds);
}

int 
xmlSecTransformIdsInit(void) {
    int ret;
    
    ret = xmlSecPtrListInitialize(xmlSecTransformIdsGet(), xmlSecTransformIdListId);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecPtrListPtrInitialize",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "xmlSecTransformIdListId");
        return(-1);
    }
    
    ret = xmlSecTransformIdsRegisterDefault();
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformIdsRegisterDefault",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
        return(-1);
    }
    
    return(0);
}

void
xmlSecTransformIdsShutdown(void) {
    xmlSecPtrListFinalize(xmlSecTransformIdsGet());
}

int 
xmlSecTransformIdsRegister(xmlSecTransformId id) {
    int ret;
        
    xmlSecAssert2(id != xmlSecTransformIdUnknown, -1);
    
    ret = xmlSecPtrListAdd(xmlSecTransformIdsGet(), (xmlSecPtr)id);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecPtrListAdd",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
        return(-1);
    }
    
    return(0);    
}

int 
xmlSecTransformIdsRegisterDefault(void) {
    if(xmlSecTransformIdsRegister(xmlSecTransformBase64Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformIdsRegister",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformBase64Id)));
	return(-1);
    }

    if(xmlSecTransformIdsRegister(xmlSecTransformEnvelopedId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformIdsRegister",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformEnvelopedId)));
	return(-1);
    }

    /* c14n methods */
    if(xmlSecTransformIdsRegister(xmlSecTransformInclC14NId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformIdsRegister",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformInclC14NId)));
	return(-1);
    }
    if(xmlSecTransformIdsRegister(xmlSecTransformInclC14NWithCommentsId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformIdsRegister",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformInclC14NWithCommentsId)));
	return(-1);
    }
    if(xmlSecTransformIdsRegister(xmlSecTransformExclC14NId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformIdsRegister",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformExclC14NId)));
	return(-1);
    }
    if(xmlSecTransformIdsRegister(xmlSecTransformExclC14NWithCommentsId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformIdsRegister",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformExclC14NWithCommentsId)));
	return(-1);
    }

    if(xmlSecTransformIdsRegister(xmlSecTransformXPathId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformIdsRegister",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformXPathId)));
	return(-1);
    }

    if(xmlSecTransformIdsRegister(xmlSecTransformXPath2Id) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformIdsRegister",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformXPath2Id)));
	return(-1);
    }

    if(xmlSecTransformIdsRegister(xmlSecTransformXPointerId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformIdsRegister",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformXPointerId)));
	return(-1);
    }

#ifndef XMLSEC_NO_XSLT
    if(xmlSecTransformIdsRegister(xmlSecTransformXsltId) < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformIdsRegister",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformXsltId)));
	return(-1);
    }
#endif /* XMLSEC_NO_XSLT */    
    
    return(0);
}

/**************************************************************************
 *
 * utils
 *
 *************************************************************************/
int 
xmlSecTransformUriTypeCheck(xmlSecTransformUriType type, const xmlChar* uri) {
    xmlSecTransformUriType uriType = 0;

    if((uri == NULL) || (xmlStrlen(uri) == 0)) {
	uriType = xmlSecTransformUriTypeEmpty;
    } else if(uri[0] == '#') {
	uriType = xmlSecTransformUriTypeSameDocument;
    } else if(xmlStrncmp(uri, BAD_CAST "file://", 7) == 0) {
	uriType = xmlSecTransformUriTypeLocal;
    } else {
	uriType = xmlSecTransformUriTypeRemote;
    }    
    return(((uriType & type) != 0) ? 1 : 0);
}

/**************************************************************************
 *
 * xmlSecTransformCtx
 *
 *************************************************************************/
xmlSecTransformCtxPtr 
xmlSecTransformCtxCreate(void) {
    xmlSecTransformCtxPtr ctx;
    int ret;
    
    /* Allocate a new xmlSecTransform and fill the fields. */
    ctx = (xmlSecTransformCtxPtr)xmlMalloc(sizeof(xmlSecTransformCtx));
    if(ctx == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlMalloc",
		    XMLSEC_ERRORS_R_MALLOC_FAILED,
		    "size=%d", sizeof(xmlSecTransformCtx)); 
	return(NULL);
    }
    
    ret = xmlSecTransformCtxInitialize(ctx);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCtxInitialize",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	xmlSecTransformCtxDestroy(ctx);
	return(NULL);
    }
    
    return(ctx);
}

void
xmlSecTransformCtxDestroy(xmlSecTransformCtxPtr ctx) {
    xmlSecAssert(ctx != NULL);
    
    xmlSecTransformCtxFinalize(ctx);
    xmlFree(ctx);
}

int 
xmlSecTransformCtxInitialize(xmlSecTransformCtxPtr ctx) {
    int ret;
    
    xmlSecAssert2(ctx != NULL, -1);
    
    memset(ctx, 0, sizeof(xmlSecTransformCtx));

    ret = xmlSecPtrListInitialize(&(ctx->enabledTransforms), xmlSecTransformIdListId);
    if(ret < 0) { 
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecPtrListInitialize",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }

    ctx->enabledUris = xmlSecTransformUriTypeAny;
    return(0);
}

void 
xmlSecTransformCtxFinalize(xmlSecTransformCtxPtr ctx) {
    xmlSecAssert(ctx != NULL);
    
    xmlSecTransformCtxReset(ctx);
    xmlSecPtrListFinalize(&(ctx->enabledTransforms));
    memset(ctx, 0, sizeof(xmlSecTransformCtx));
}

void 
xmlSecTransformCtxReset(xmlSecTransformCtxPtr ctx) {
    xmlSecTransformPtr transform, tmp;    
    
    xmlSecAssert(ctx != NULL);

    ctx->result = NULL;
    ctx->status = xmlSecTransformStatusNone;
    
    /* destroy uri */
    if(ctx->uri != NULL) {
	xmlFree(ctx->uri);
	ctx->uri = NULL;
    }
    if(ctx->xptrExpr != NULL) {
	xmlFree(ctx->xptrExpr);
	ctx->xptrExpr = NULL;
    }
    
    /* destroy transforms chain */
    for(transform = ctx->first; transform != NULL; transform = tmp) {
	tmp = transform->next;
	xmlSecTransformDestroy(transform);
    }
    ctx->first = ctx->last = NULL;
}

int 
xmlSecTransformCtxCopyUserPref(xmlSecTransformCtxPtr dst, xmlSecTransformCtxPtr src) {
    int ret;
    
    xmlSecAssert2(dst != NULL, -1);
    xmlSecAssert2(src != NULL, -1);
    
    dst->userData 	 = src->userData;  
    dst->flags		 = src->flags;  
    dst->flags2		 = src->flags2;  
    dst->enabledUris	 = src->enabledUris;
    dst->preExecCallback = src->preExecCallback;
    
    ret = xmlSecPtrListCopy(&(dst->enabledTransforms), &(src->enabledTransforms));
    if(ret < 0) { 
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecPtrListCCCopy",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    
    return(0);
}

int 
xmlSecTransformCtxAppend(xmlSecTransformCtxPtr ctx, xmlSecTransformPtr transform) {
    int ret;
    
    xmlSecAssert2(ctx != NULL, -1);    
    xmlSecAssert2(ctx->status == xmlSecTransformStatusNone, -1);
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);

    if(ctx->last != NULL) {
	ret = xmlSecTransformConnect(ctx->last, transform, ctx);
	if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"xmlSecTransformConnect",	    
		        XMLSEC_ERRORS_R_XMLSEC_FAILED,
			"name=%s",
			xmlSecErrorsSafeString(xmlSecTransformGetName(transform)));
	    return(-1);
	}
    } else {
	xmlSecAssert2(ctx->first == NULL, -1);
	ctx->first = transform;
    }
    ctx->last = transform;

    return(0);
}

int 
xmlSecTransformCtxPrepend(xmlSecTransformCtxPtr ctx, xmlSecTransformPtr transform) {
    int ret;
    
    xmlSecAssert2(ctx != NULL, -1);
    xmlSecAssert2(ctx->status == xmlSecTransformStatusNone, -1);
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);

    if(ctx->first != NULL) {
	ret = xmlSecTransformConnect(transform, ctx->first, ctx);
	if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"xmlSecTransformConnect",	    
		        XMLSEC_ERRORS_R_XMLSEC_FAILED,
			"name=%s",
			xmlSecErrorsSafeString(xmlSecTransformGetName(transform)));
	    return(-1);
	}
    } else {
	xmlSecAssert2(ctx->last == NULL, -1);
	ctx->last = transform;
    }
    ctx->first = transform;

    return(0);
}

xmlSecTransformPtr 
xmlSecTransformCtxCreateAndAppend(xmlSecTransformCtxPtr ctx, xmlSecTransformId id) {
    xmlSecTransformPtr transform;
    int ret;
    
    xmlSecAssert2(ctx != NULL, NULL);
    xmlSecAssert2(ctx->status == xmlSecTransformStatusNone, NULL);
    xmlSecAssert2(id != xmlSecTransformIdUnknown, NULL);

    transform = xmlSecTransformCreate(id);
    if(!xmlSecTransformIsValid(transform)) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCreate",		    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "transform=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(id)));
	return(NULL);
    }

    ret = xmlSecTransformCtxAppend(ctx, transform);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCtxAppend",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)));
	xmlSecTransformDestroy(transform);
	return(NULL);
    }

    return(transform);
}

xmlSecTransformPtr 
xmlSecTransformCtxCreateAndPrepend(xmlSecTransformCtxPtr ctx, xmlSecTransformId id) {
    xmlSecTransformPtr transform;
    int ret;
    
    xmlSecAssert2(ctx != NULL, NULL);
    xmlSecAssert2(ctx->status == xmlSecTransformStatusNone, NULL);
    xmlSecAssert2(id != xmlSecTransformIdUnknown, NULL);

    transform = xmlSecTransformCreate(id);
    if(!xmlSecTransformIsValid(transform)) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCreate",		    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "transform=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(id)));
	return(NULL);
    }

    ret = xmlSecTransformCtxPrepend(ctx, transform);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCtxPrepend",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)));
	xmlSecTransformDestroy(transform);
	return(NULL);
    }

    return(transform);
}


xmlSecTransformPtr
xmlSecTransformCtxNodeRead(xmlSecTransformCtxPtr ctx, xmlNodePtr node, 
			   xmlSecTransformUsage usage) {
    xmlSecTransformPtr transform;
    int ret;
    
    xmlSecAssert2(ctx != NULL, NULL);
    xmlSecAssert2(ctx->status == xmlSecTransformStatusNone, NULL);
    xmlSecAssert2(node != NULL, NULL);
    
    transform = xmlSecTransformNodeRead(node, usage, ctx);
    if(transform == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformNodeRead",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecNodeGetName(node))); 
	return(NULL);
    }
    
    ret = xmlSecTransformCtxAppend(ctx, transform);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCtxAppend",	    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)));
	xmlSecTransformDestroy(transform);
	return(NULL);
    }
    
    return(transform);
}

int 
xmlSecTransformCtxNodesListRead(xmlSecTransformCtxPtr ctx, xmlNodePtr node, xmlSecTransformUsage usage) {
    xmlSecTransformPtr transform;
    xmlNodePtr cur;
    int ret;
        
    xmlSecAssert2(ctx != NULL, -1);
    xmlSecAssert2(ctx->status == xmlSecTransformStatusNone, -1);
    xmlSecAssert2(node != NULL, -1);
    
    cur = xmlSecGetNextElementNode(node->children);
    while((cur != NULL) && xmlSecCheckNodeName(cur, xmlSecNodeTransform, xmlSecDSigNs)) {
	transform = xmlSecTransformNodeRead(cur, usage, ctx);
	if(transform == NULL) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"xmlSecTransformNodeRead",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			"node=%s",
			xmlSecErrorsSafeString(xmlSecNodeGetName(cur)));
	    return(-1);
	}
	
	ret = xmlSecTransformCtxAppend(ctx, transform); 
	if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"xmlSecTransformCtxAppend",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			"node=%s",
			xmlSecErrorsSafeString(xmlSecNodeGetName(cur)));
	    xmlSecTransformDestroy(transform);
	    return(-1);
	}	
	cur = xmlSecGetNextElementNode(cur->next);
    }

    if(cur != NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    xmlSecErrorsSafeString(xmlSecNodeGetName(cur)),
		    XMLSEC_ERRORS_R_UNEXPECTED_NODE,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }    
    return(0);
}

/**
 * xmlSecTransformCtxSetUri:
 *
 * Parses uri and adds xpointer transforms if required.
 *
 * The following examples demonstrate what the URI attribute identifies and
 * how it is dereferenced 
 * (http://www.w3.org/TR/xmldsig-core/#sec-ReferenceProcessingModel):
 *
 * - URI="http://example.com/bar.xml"
 * identifies the octets that represent the external resource 
 * 'http://example.com/bar.xml', that is probably an XML document given 
 * its file extension. 
 *
 * - URI="http://example.com/bar.xml#chapter1"
 * identifies the element with ID attribute value 'chapter1' of the 
 * external XML resource 'http://example.com/bar.xml', provided as an 
 * octet stream. Again, for the sake of interoperability, the element 
 * identified as 'chapter1' should be obtained using an XPath transform 
 * rather than a URI fragment (barename XPointer resolution in external 
 * resources is not REQUIRED in this specification). 
 *
 * - URI=""
 * identifies the node-set (minus any comment nodes) of the XML resource 
 * containing the signature 
 *
 * - URI="#chapter1"
 * identifies a node-set containing the element with ID attribute value 
 * 'chapter1' of the XML resource containing the signature. XML Signature 
 * (and its applications) modify this node-set to include the element plus 
 * all descendents including namespaces and attributes -- but not comments.
 *
 */
int
xmlSecTransformCtxSetUri(xmlSecTransformCtxPtr ctx, const xmlChar* uri, xmlNodePtr hereNode) {
    xmlSecTransformPtr transform;
    xmlSecNodeSetType nodeSetType = xmlSecNodeSetTree;
    const xmlChar* xptr;
    xmlChar* buf = NULL;
    int ret;
    
    xmlSecAssert2(ctx != NULL, -1);
    xmlSecAssert2(ctx->uri == NULL, -1);
    xmlSecAssert2(ctx->xptrExpr == NULL, -1);
    xmlSecAssert2(ctx->status == xmlSecTransformStatusNone, -1);
    xmlSecAssert2(hereNode != NULL, -1);

    /* check uri */
    if(xmlSecTransformUriTypeCheck(ctx->enabledUris, uri) != 1) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    NULL,
		    XMLSEC_ERRORS_R_INVALID_URI_TYPE,
		    "uri=\"%s\"", xmlSecErrorsSafeString(uri));
	return(-1);
    }

    /* is it an empty uri? */    
    if((uri == NULL) || (xmlStrlen(uri) == 0)) {
	return(0);
    }

    /* do we have barename or full xpointer? */
    xptr = (const xmlChar *)strchr((const char*)uri, '#');
    if(xptr == NULL){
        ctx->uri = xmlStrdup(uri);
	if(ctx->uri == NULL) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"xmlStrdup",
			XMLSEC_ERRORS_R_MALLOC_FAILED,
			"size=%d", xmlStrlen(uri)); 
	    return(-1);
	}
	/* we are done */
	return(0);
    } else if(strcmp(uri, "#xpointer(/)") == 0) {
        ctx->xptrExpr = xmlStrdup(uri);
	if(ctx->xptrExpr == NULL) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"xmlStrdup",
			XMLSEC_ERRORS_R_MALLOC_FAILED,
			"size=%d", xmlStrlen(uri)); 
	    return(-1);
	}
	/* we are done */
	return(0);	
    }
    
    ctx->uri = xmlStrndup(uri, xptr - uri);
    if(ctx->uri == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlStrndup",
		    XMLSEC_ERRORS_R_MALLOC_FAILED,
		    "size=%d", xptr - uri); 
	return(-1);
    }

    ctx->xptrExpr = xmlStrdup(xptr);
    if(ctx->xptrExpr == NULL) {
        xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlStrdup",
		    XMLSEC_ERRORS_R_MALLOC_FAILED,
		    "size=%d", xmlStrlen(xptr)); 
	return(-1);
    }

    /* we need to create XPonter transform to execute expr */
    xmlSecAssert2(xptr != NULL, -1);
    transform = xmlSecTransformCtxCreateAndPrepend(ctx, xmlSecTransformXPointerId);
    if(!xmlSecTransformIsValid(transform)) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCtxCreateAndPrepend", 
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "transform=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformXPointerId)));
	return(-1);
    }
    
    /* do we have barename or full xpointer? */
    if((strncmp(xptr, "#xpointer(", 10) == 0) || (strncmp(xptr, "#xmlns(", 7) == 0)) {
	++xptr;
	nodeSetType = xmlSecNodeSetTree;
    } else {
	static const char tmpl[] = "xpointer(id(\'%s\'))";
	size_t size;
	
	/* we need to add "xpointer(id('..')) because otherwise we have 
	 * problems with numeric ("111" and so on) and other "strange" ids */
	size = xmlStrlen(BAD_CAST tmpl) + xmlStrlen(xptr) + 2;
	buf = (xmlChar*)xmlMalloc(size * sizeof(xmlChar));
	if(buf == NULL) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"xmlMalloc",
			XMLSEC_ERRORS_R_MALLOC_FAILED,
			"size=%d", size);
	    return(-1);	    
	}
	sprintf((char*)buf, tmpl, xptr + 1);
	xptr = buf;
	nodeSetType = xmlSecNodeSetTreeWithoutComments;
    }
    
    /* finally initialize our xpointer transform */
    ret = xmlSecTransformXPointerSetExpr(transform, xptr, nodeSetType, hereNode);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformXPointerSetExpr",  
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "name=%s",
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)));
	if(buf != NULL) {
	    xmlFree(buf);
	}
	return(-1);
    }
    if(buf != NULL) {
	xmlFree(buf);
    }
    
    return(0);
}

int 
xmlSecTransformCtxPrepare(xmlSecTransformCtxPtr ctx, xmlSecTransformDataType inputDataType) {
    xmlSecTransformDataType firstType;
    xmlSecTransformPtr transform;
    int ret;
    
    xmlSecAssert2(ctx != NULL, -1);
    xmlSecAssert2(ctx->result == NULL, -1);
    xmlSecAssert2(ctx->status == xmlSecTransformStatusNone, -1);
    
    /* add binary buffer to store result */
    transform = xmlSecTransformCtxCreateAndAppend(ctx, xmlSecTransformMemBufId);
    if(!xmlSecTransformIsValid(transform)) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCreate",		    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "transform=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformMemBufId)));
	return(-1);
    }
    ctx->result = xmlSecTransformMemBufGetBuffer(transform);
    if(ctx->result == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformMemBufGetBuffer",  
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "transform=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformMemBufId)));
	return(-1);
    }    

    firstType = xmlSecTransformGetDataType(ctx->first, xmlSecTransformModePush, ctx);
    if(((firstType & xmlSecTransformDataTypeBin) == 0) &&
       ((inputDataType & xmlSecTransformDataTypeBin) != 0)) {
	
        /* need to add parser transform */
	transform = xmlSecTransformCtxCreateAndPrepend(ctx, xmlSecTransformXmlParserId);
	if(transform == NULL) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"xmlSecTransformCtxCreateAndPrepend",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			"transform=%s",
			xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformXmlParserId)));
	    return(-1);
	}
    } else if(((firstType & xmlSecTransformDataTypeXml) == 0) &&
       ((inputDataType & xmlSecTransformDataTypeXml) != 0)) {

	/* need to add c14n transform */
	transform = xmlSecTransformCtxCreateAndPrepend(ctx, xmlSecTransformInclC14NId);
	if(transform == NULL) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"xmlSecTransformCtxCreateAndPrepend",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			"transform=%s",
			xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformInclC14NId)));
	    return(-1);
	}
    }

    /* finally let application a chance to verify that it's ok to execte
     * this transforms chain */
    if(ctx->preExecCallback != NULL) {
	ret = (ctx->preExecCallback)(ctx);
	if(ret < 0) {
    	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"ctx->preExecCallback", 
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			XMLSEC_ERRORS_NO_MESSAGE);
    	    return(-1);
	}	
    }

    ctx->status = xmlSecTransformStatusWorking;    
    return(0);
}

int
xmlSecTransformCtxBinaryExecute(xmlSecTransformCtxPtr ctx, const unsigned char* data, size_t dataSize) {
    int ret;
        
    xmlSecAssert2(ctx != NULL, -1);
    xmlSecAssert2(ctx->result == NULL, -1);
    xmlSecAssert2(ctx->status == xmlSecTransformStatusNone, -1);
    xmlSecAssert2(data != NULL, -1);
    xmlSecAssert2(dataSize > 0, -1);

    /* we should not have uri stored in ctx */
    xmlSecAssert2(ctx->uri == NULL, -1);
    
    ret = xmlSecTransformCtxPrepare(ctx, xmlSecTransformDataTypeBin);
    if(ret < 0) {
        xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCtxPrepare", 
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "type=bin");
    	return(-1);
    }	
    
    ret = xmlSecTransformPushBin(ctx->first, data, dataSize, 1, ctx);
    if(ret < 0) {
    	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCtxPushBin", 
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "dataSize=%d", dataSize);
    	return(-1);
    }

    ctx->status = xmlSecTransformStatusFinished;
    return(0);    
}

int 
xmlSecTransformCtxUriExecute(xmlSecTransformCtxPtr ctx, const xmlChar* uri) {
    xmlSecTransformPtr uriTransform;
    int ret;
        
    xmlSecAssert2(ctx != NULL, -1);
    xmlSecAssert2(ctx->status == xmlSecTransformStatusNone, -1);
    xmlSecAssert2(uri != NULL, -1);

    /* we should not execute transform for a different uri */
    xmlSecAssert2((ctx->uri == NULL) || (uri == ctx->uri) || xmlStrEqual(uri, ctx->uri), -1);
    
    uriTransform = xmlSecTransformCtxCreateAndPrepend(ctx, xmlSecTransformInputURIId);
    if(uriTransform == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCtxCreateAndPrepend",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "transform=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(xmlSecTransformInputURIId)));
	return(-1);
    }
    
    ret = xmlSecTransformInputURIOpen(uriTransform, uri);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformInputURIOpen",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "uri=%s",
		    xmlSecErrorsSafeString(uri));
	return(-1);
    }

    /* we do not need to do something special for this transofrm */
    ret = xmlSecTransformCtxPrepare(ctx, xmlSecTransformDataTypeUnknown);
    if(ret < 0) {
        xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCtxPrepare", 
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "type=bin");
    	return(-1);
    }	
    
    /* Now we have a choice: we either can push from first transform or pop 
     * from last. Our C14N transforms prefers push, so push data!
     */
    ret = xmlSecTransformPump(uriTransform, uriTransform->next, ctx);     
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformPump",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "uri=%s",
		    xmlSecErrorsSafeString(uri));
	return(-1);
    }
     
    ctx->status = xmlSecTransformStatusFinished;
    return(0);
}

int
xmlSecTransformCtxXmlExecute(xmlSecTransformCtxPtr ctx, xmlSecNodeSetPtr nodes) {
    int ret;
        
    xmlSecAssert2(ctx != NULL, -1);
    xmlSecAssert2(ctx->result == NULL, -1);
    xmlSecAssert2(ctx->status == xmlSecTransformStatusNone, -1);
    xmlSecAssert2(nodes != NULL, -1);
    
    xmlSecAssert2((ctx->uri == NULL) || (xmlStrlen(ctx->uri) == 0), -1); 

    ret = xmlSecTransformCtxPrepare(ctx, xmlSecTransformDataTypeXml);
    if(ret < 0) {
    	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCtxPrepare", 
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "type=xml");
    	return(-1);
    }	

    /* it's better to do push than pop because all XML transform
     * just don't care and c14n likes push more than pop */
    ret = xmlSecTransformPushXml(ctx->first, nodes, ctx);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformPushXml", 
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "transform=%s",
		    xmlSecErrorsSafeString(xmlSecTransformGetName(ctx->first)));
	return(-1);
    }

    ctx->status = xmlSecTransformStatusFinished;
    return(0);
}

int
xmlSecTransformCtxExecute(xmlSecTransformCtxPtr ctx, xmlDocPtr doc) {
    int ret;
        
    xmlSecAssert2(ctx != NULL, -1);
    xmlSecAssert2(ctx->result == NULL, -1);
    xmlSecAssert2(ctx->status == xmlSecTransformStatusNone, -1);
    xmlSecAssert2(doc != NULL, -1);
    
    if((ctx->uri == NULL) || (xmlStrlen(ctx->uri) == 0)) {
	xmlSecNodeSetPtr nodes;
        
	if((ctx->xptrExpr != NULL) && (xmlStrlen(ctx->xptrExpr) > 0)){
	    /* our xpointer transform takes care of providing correct nodes set */
	    nodes = xmlSecNodeSetCreate(doc, NULL, xmlSecNodeSetNormal);
	    if(nodes == NULL) {
		xmlSecError(XMLSEC_ERRORS_HERE,
			    NULL,
			    "xmlSecNodeSetCreate", 
			    XMLSEC_ERRORS_R_XMLSEC_FAILED,
			    XMLSEC_ERRORS_NO_MESSAGE);
	        return(-1);
	    }
	
	} else {
	    /* we do not want to have comments for empty URI */
	    nodes = xmlSecNodeSetGetChildren(doc, xmlDocGetRootElement(doc), 0, 0);
	    if(nodes == NULL) {
		xmlSecError(XMLSEC_ERRORS_HERE,
			    NULL,
			    "xmlSecNodeSetGetChildren", 
			    XMLSEC_ERRORS_R_XMLSEC_FAILED,
			    XMLSEC_ERRORS_NO_MESSAGE);
	        return(-1);
	    }
	}
	ret = xmlSecTransformCtxXmlExecute(ctx, nodes);
	if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"xmlSecTransformCtxXmlExecute", 
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			XMLSEC_ERRORS_NO_MESSAGE);
	    xmlSecNodeSetDestroy(nodes);
	    return(-1);
	}
	/* TODO: don't destroy nodes here */
	xmlSecNodeSetDestroy(nodes);
    } else {
	ret = xmlSecTransformCtxUriExecute(ctx, ctx->uri);
	if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"xmlSecTransformCtxUriExecute", 
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			XMLSEC_ERRORS_NO_MESSAGE);
	    return(-1);
	}
    } 
    
    return(0);
}



void 
xmlSecTransformCtxDebugDump(xmlSecTransformCtxPtr ctx, FILE* output) {
    xmlSecTransformPtr transform;    
    
    xmlSecAssert(ctx != NULL);
    xmlSecAssert(output != NULL);

    fprintf(output, "== TRANSFORMS CTX (status=%d)\n", ctx->status);    
    fprintf(output, "=== uri: %s\n", 
	    (ctx->uri != NULL) ? ctx->uri : BAD_CAST "NULL");    
    fprintf(output, "=== uri xpointer expr: %s\n", 
	    (ctx->xptrExpr != NULL) ? ctx->xptrExpr : BAD_CAST "NULL");    
    for(transform = ctx->first; transform != NULL; transform = transform->next) {
	xmlSecTransformDebugDump(transform, output);
    }
}

void 
xmlSecTransformCtxDebugXmlDump(xmlSecTransformCtxPtr ctx, FILE* output) {
    xmlSecTransformPtr transform;    
    
    xmlSecAssert(ctx != NULL);
    xmlSecAssert(output != NULL);
 
    fprintf(output, "<TransformCtx status=\"%d\" uri=\"%s\" uriXPointer=\"%s\" >\n", 
			ctx->status, 
			(ctx->uri != NULL) ? ctx->uri : BAD_CAST "NULL",    
			(ctx->xptrExpr != NULL) ? ctx->xptrExpr : BAD_CAST "NULL");    
    for(transform = ctx->first; transform != NULL; transform = transform->next) {
	xmlSecTransformDebugXmlDump(transform, output);
    }
    fprintf(output, "</TransformCtx>\n");    
}

/**************************************************************************
 *
 * xmlSecTransform
 *
 *************************************************************************/
/**
 * xmlSecTransformCreate:
 * @id: the transform id to create.
 *
 * Creates new transform from the transform id object.
 *
 * Returns the pointer to newly created #xmlSecTransform structure
 * or NULL if an error occurs.
 */ 
xmlSecTransformPtr	
xmlSecTransformCreate(xmlSecTransformId id) {
    xmlSecTransformPtr transform;
    int ret;
    
    xmlSecAssert2(id != NULL, NULL);
    xmlSecAssert2(id->klassSize >= sizeof(xmlSecTransformKlass), NULL);
    xmlSecAssert2(id->objSize >= sizeof(xmlSecTransform), NULL);
    xmlSecAssert2(id->name != NULL, NULL);
        
    /* Allocate a new xmlSecTransform and fill the fields. */
    transform = (xmlSecTransformPtr)xmlMalloc(id->objSize);
    if(transform == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlMalloc",
		    XMLSEC_ERRORS_R_MALLOC_FAILED,
		    "size=%d", id->objSize); 
	return(NULL);
    }
    memset(transform, 0, id->objSize);    
    transform->id = id;
    
    if(id->initialize != NULL) {
	ret = (id->initialize)(transform);
        if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
			"id->initialize",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			XMLSEC_ERRORS_NO_MESSAGE);
	    xmlSecTransformDestroy(transform);
	    return(NULL);
	}
    }

    ret = xmlSecBufferInitialize(&(transform->inBuf), 0);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    "xmlSecBufferInitialize",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "size=%d", 0);
	xmlSecTransformDestroy(transform);
	return(NULL);	
    }

    ret = xmlSecBufferInitialize(&(transform->outBuf), 0);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    "xmlSecBufferInitialize",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "size=%d", 0);
	xmlSecTransformDestroy(transform);
	return(NULL);	
    }
    
    return(transform);
}

/**
 * xmlSecTransformDestroy:
 * @transform: the pointer to #xmlSecTransform structure.
 *
 * Destroys transform by calling appropriate transform specific function.
 */
void
xmlSecTransformDestroy(xmlSecTransformPtr transform) {
    xmlSecAssert(xmlSecTransformIsValid(transform));
    xmlSecAssert(transform->id->objSize > 0);
    
    /* first need to remove ourselves from chain */
    xmlSecTransformRemove(transform);

    xmlSecBufferFinalize(&(transform->inBuf));
    xmlSecBufferFinalize(&(transform->outBuf));

    /* we never destroy input nodes, output nodes
     * are destroyed if and only if they are different
     * from input nodes 
     */
    if((transform->outNodes != NULL) && (transform->outNodes != transform->inNodes)) {
	xmlSecNodeSetDestroy(transform->outNodes);
    }
    if(transform->id->finalize != NULL) { 
	(transform->id->finalize)(transform);
    }
    memset(transform, 0, transform->id->objSize);
    xmlFree(transform);
}

/** 
 * xmlSecTransformNodeRead:
 * @transform: the pointer to #xmlSecTransform structure.
 * @node: the pointer to the <dsig:Transform> node.
 *
 * Reads transform from the @node as follows:
 *    1) reads "Algorithm" attribute;
 *    2) checks the list of known algorithms;
 *    3) calls transform create method;
 *    4) calls transform read transform node method.
 *
 * Returns 0 on success or a negative value otherwise.
 */
xmlSecTransformPtr
xmlSecTransformNodeRead(xmlNodePtr node, xmlSecTransformUsage usage, xmlSecTransformCtxPtr transformCtx) {
    xmlSecTransformPtr transform;
    xmlSecTransformId id;
    xmlChar *href;
    int ret;

    xmlSecAssert2(node != NULL, NULL);
    xmlSecAssert2(transformCtx != NULL, NULL);

    href = xmlGetProp(node, xmlSecAttrAlgorithm);
    if(href == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlGetProp",
		    XMLSEC_ERRORS_R_INVALID_NODE_ATTRIBUTE,
		    "node=%s",
		    xmlSecErrorsSafeString(xmlSecAttrAlgorithm));
	return(NULL);		
    }
    
    id = xmlSecTransformIdListFindByHref(xmlSecTransformIdsGet(), href, usage);    
    if(id == xmlSecTransformIdUnknown) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformIdsListFindByHref",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "href=\"%s\"", 
		    xmlSecErrorsSafeString(href));
	xmlFree(href);
	return(NULL);		
    }

    /* check with enabled transforms list */
    if((xmlSecPtrListGetSize(&(transformCtx->enabledTransforms)) > 0) &&
       (xmlSecTransformIdListFind(&(transformCtx->enabledTransforms), id) != 1)) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(id)),
		    XMLSEC_ERRORS_R_TRANSFORM_DISABLED,
		    "href=%s",
		    xmlSecErrorsSafeString(href));
	xmlFree(href);
	return(NULL);
    }
        
    transform = xmlSecTransformCreate(id);
    if(!xmlSecTransformIsValid(transform)) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlSecTransformCreate",		    
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "transform=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(id)));
	xmlFree(href);
	return(NULL);		
    }

    if(transform->id->readNode != NULL) {
	ret = transform->id->readNode(transform, node, transformCtx);
        if(ret < 0) {
    	    xmlSecError(XMLSEC_ERRORS_HERE,
			NULL,
			"id->readNode",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			"transform=%s",
			xmlSecErrorsSafeString(xmlSecTransformGetName(transform)));
	    xmlSecTransformDestroy(transform);
	    xmlFree(href);
	    return(NULL);		
	}
    }

    /* finally remember the transform node */    
    transform->hereNode = node;
    xmlFree(href);   
    return(transform);
}

int 
xmlSecTransformPump(xmlSecTransformPtr left, xmlSecTransformPtr right, xmlSecTransformCtxPtr transformCtx) {
    xmlSecTransformDataType leftType;
    xmlSecTransformDataType rightType;
    int ret;
    
    xmlSecAssert2(xmlSecTransformIsValid(left), -1);
    xmlSecAssert2(xmlSecTransformIsValid(right), -1);
    xmlSecAssert2(transformCtx != NULL, -1);
    
    leftType = xmlSecTransformGetDataType(left, xmlSecTransformModePop, transformCtx);
    rightType = xmlSecTransformGetDataType(right, xmlSecTransformModePush, transformCtx);

    if(((leftType & xmlSecTransformDataTypeXml) != 0) && 
       ((rightType & xmlSecTransformDataTypeXml) != 0)) {
       
       xmlSecNodeSetPtr nodes = NULL;

       ret = xmlSecTransformPopXml(left, &nodes, transformCtx);
       if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			xmlSecErrorsSafeString(xmlSecTransformGetName(left)),
			"xmlSecTransformPopXml",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			XMLSEC_ERRORS_NO_MESSAGE);
	    return(-1);
       }

       ret = xmlSecTransformPushXml(right, nodes, transformCtx);
       if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			xmlSecErrorsSafeString(xmlSecTransformGetName(right)),
			"xmlSecTransformPushXml",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			XMLSEC_ERRORS_NO_MESSAGE);
	    return(-1);
       }
    }  else if(((leftType & xmlSecTransformDataTypeBin) != 0) && 
    	       ((rightType & xmlSecTransformDataTypeBin) != 0)) {	
	unsigned char buf[XMLSEC_TRANSFORM_BINARY_CHUNK];
	size_t bufSize;
	int final;
	
	do {
	    ret = xmlSecTransformPopBin(left, buf, sizeof(buf), &bufSize, transformCtx);
    	    if(ret < 0) {
		xmlSecError(XMLSEC_ERRORS_HERE,
			    xmlSecErrorsSafeString(xmlSecTransformGetName(left)),
			    "xmlSecTransformPopBin",
			    XMLSEC_ERRORS_R_XMLSEC_FAILED,
			    XMLSEC_ERRORS_NO_MESSAGE);
		return(-1);
	    }
	    final = (bufSize == 0) ? 1 : 0;
	    ret = xmlSecTransformPushBin(right, buf, bufSize, final, transformCtx);
    	    if(ret < 0) {
		xmlSecError(XMLSEC_ERRORS_HERE,
			    xmlSecErrorsSafeString(xmlSecTransformGetName(right)),
			    "xmlSecTransformPushBin",
			    XMLSEC_ERRORS_R_XMLSEC_FAILED,
			    XMLSEC_ERRORS_NO_MESSAGE);
		return(-1);
	    }
	} while(final == 0);
    } else {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(left)),
		    xmlSecErrorsSafeString(xmlSecTransformGetName(right)),
		    XMLSEC_ERRORS_R_INVALID_TRANSFORM,
		    "transforms input/output data formats do not match");
    }
    return(0);
}


/**
 * xmlSecTransformSetKey:
 * @transform: the pointer to #xmlSecTransform structure.
 * @key: the pointer to #xmlSecKey structure. 
 *
 * Sets the key for binary transform (wrapper for transform specific 
 * setKey() method).
 *
 * Returns 0 on success or a negative value otherwise.
 */
int
xmlSecTransformSetKey(xmlSecTransformPtr transform, xmlSecKeyPtr key) {
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(key != NULL, -1);
        
    if(transform->id->setKey != NULL) {
	return((transform->id->setKey)(transform, key));
    }
    return(0);
}

/**
 * xmlSecTransformSetKeyReq:
 * @transform: the pointer to #xmlSecTransform structure.
 * @keyInfoCtx: the pointer to #xmlSecKeyInfoCtx structure. 
 *
 * Sets the key for binary transform (wrapper for transform specific 
 * setKeyReq() method).
 *
 * Returns 0 on success or a negative value otherwise.
 */
int
xmlSecTransformSetKeyReq(xmlSecTransformPtr transform, xmlSecKeyReqPtr keyReq) {
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(keyReq != NULL, -1);
        
    keyReq->keyId   	= xmlSecKeyDataIdUnknown;
    keyReq->keyType 	= xmlSecKeyDataTypeUnknown;
    keyReq->keyUsage	= xmlSecKeyUsageAny;
    keyReq->keyBitsSize	= 0;
        
    if(transform->id->setKeyReq != NULL) {
	return((transform->id->setKeyReq)(transform, keyReq));
    }
    return(0);
}

int 
xmlSecTransformVerify(xmlSecTransformPtr transform, const unsigned char* data,
		    size_t dataSize, xmlSecTransformCtxPtr transformCtx) {
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(transform->id->verify != NULL, -1);
    xmlSecAssert2(transformCtx != NULL, -1);

    return((transform->id->verify)(transform, data, dataSize, transformCtx));
}

int 
xmlSecTransformVerifyNodeContent(xmlSecTransformPtr transform, xmlNodePtr node,
				 xmlSecTransformCtxPtr transformCtx) {
    xmlSecBuffer buffer;
    int ret;
    
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(node != NULL, -1);
    xmlSecAssert2(transformCtx != NULL, -1);
    
    ret = xmlSecBufferInitialize(&buffer, 0);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    "xmlSecBufferInitialize",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    
    ret = xmlSecBufferBase64NodeContentRead(&buffer, node);
    if((ret < 0) || (xmlSecBufferGetData(&buffer) == NULL)) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    "xmlSecBufferBase64NodeContentRead",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	xmlSecBufferFinalize(&buffer);
	return(-1);
    }
    
    ret = xmlSecTransformVerify(transform, xmlSecBufferGetData(&buffer),
				xmlSecBufferGetSize(&buffer), transformCtx);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    "xmlSecTransformVerify",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	xmlSecBufferFinalize(&buffer);
	return(-1);
    }

    xmlSecBufferFinalize(&buffer);
    return(0);
}

xmlSecTransformDataType	
xmlSecTransformGetDataType(xmlSecTransformPtr transform, xmlSecTransformMode mode, 
		    xmlSecTransformCtxPtr transformCtx) {
    xmlSecAssert2(xmlSecTransformIsValid(transform), xmlSecTransformDataTypeUnknown);
    xmlSecAssert2(transform->id->getDataType != NULL, xmlSecTransformDataTypeUnknown);
    
    return((transform->id->getDataType)(transform, mode, transformCtx));    
}

int 
xmlSecTransformPushBin(xmlSecTransformPtr transform, const unsigned char* data,
		    size_t dataSize, int final, xmlSecTransformCtxPtr transformCtx) {
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(transform->id->pushBin != NULL, -1);
    xmlSecAssert2(transformCtx != NULL, -1);
    
    return((transform->id->pushBin)(transform, data, dataSize, final, transformCtx));    
}

int 
xmlSecTransformPopBin(xmlSecTransformPtr transform, unsigned char* data,
		    size_t maxDataSize, size_t* dataSize, xmlSecTransformCtxPtr transformCtx) {
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(transform->id->popBin != NULL, -1);
    xmlSecAssert2(data != NULL, -1);
    xmlSecAssert2(dataSize != NULL, -1);
    xmlSecAssert2(transformCtx != NULL, -1);

    return((transform->id->popBin)(transform, data, maxDataSize, dataSize, transformCtx));    
}

int 
xmlSecTransformPushXml(xmlSecTransformPtr transform, xmlSecNodeSetPtr nodes,
		    xmlSecTransformCtxPtr transformCtx) {
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(transform->id->pushXml != NULL, -1);
    xmlSecAssert2(transformCtx != NULL, -1);

    return((transform->id->pushXml)(transform, nodes, transformCtx));    
}

int 
xmlSecTransformPopXml(xmlSecTransformPtr transform, xmlSecNodeSetPtr* nodes,
		    xmlSecTransformCtxPtr transformCtx) {
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(transform->id->popXml != NULL, -1);
    xmlSecAssert2(transformCtx != NULL, -1);

    return((transform->id->popXml)(transform, nodes, transformCtx));    
}

int 
xmlSecTransformExecute(xmlSecTransformPtr transform, int last, xmlSecTransformCtxPtr transformCtx) {
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(transform->id->execute != NULL, -1);
    xmlSecAssert2(transformCtx != NULL, -1);

    return((transform->id->execute)(transform, last, transformCtx));
}

void 
xmlSecTransformDebugDump(xmlSecTransformPtr transform, FILE* output) {
    xmlSecAssert(xmlSecTransformIsValid(transform));
    xmlSecAssert(output != NULL);
    
    fprintf(output, "=== Transform: %s (href=%s)\n",
		xmlSecErrorsSafeString(transform->id->name),
		xmlSecErrorsSafeString(transform->id->href));
}

void 
xmlSecTransformDebugXmlDump(xmlSecTransformPtr transform, FILE* output) {
    xmlSecAssert(xmlSecTransformIsValid(transform));
    xmlSecAssert(output != NULL);

    fprintf(output, "<Transform name=\"%s\" href=\"%s\" />\n",
		xmlSecErrorsSafeString(transform->id->name),
		xmlSecErrorsSafeString(transform->id->href));
}

/************************************************************************
 *
 * Operations on transforms chain
 *
 ************************************************************************/ 
/**
 * xmlSecTransformConnect:
 * 
 * @left:
 * @right:
 *
 * If the data object is a node-set and the next transform requires octets, 
 * the signature application MUST attempt to convert the node-set to an octet 
 * stream using Canonical XML [XML-C14N].  
 *
 * The story is different if the right transform is base64 decode:
 *
 * http://www.w3.org/TR/xmldsig-core/#sec-Base-64
 *
 * This transform requires an octet stream for input. If an XPath node-set 
 * (or sufficiently functional alternative) is given as input, then it is 
 * converted to an octet stream by performing operations logically equivalent 
 * to 1) applying an XPath transform with expression self::text(), then 2) 
 * taking the string-value of the node-set. Thus, if an XML element is 
 * identified by a barename XPointer in the Reference URI, and its content 
 * consists solely of base64 encoded character data, then this transform 
 * automatically strips away the start and end tags of the identified element 
 * and any of its descendant elements as well as any descendant comments and 
 * processing instructions. The output of this transform is an octet stream.
 */
int 
xmlSecTransformConnect(xmlSecTransformPtr left, xmlSecTransformPtr right, 
		       xmlSecTransformCtxPtr transformCtx) {
    xmlSecTransformDataType leftType;
    xmlSecTransformDataType rightType;
    xmlSecTransformId middleId;
    xmlSecTransformPtr middle;
        
    xmlSecAssert2(xmlSecTransformIsValid(left), -1);
    xmlSecAssert2(xmlSecTransformIsValid(right), -1);
    xmlSecAssert2(transformCtx != NULL, -1);

    leftType = xmlSecTransformGetDataType(left, xmlSecTransformModePop, transformCtx);
    rightType = xmlSecTransformGetDataType(right, xmlSecTransformModePush, transformCtx);

    /* happy case first: nothing need to be done */    
    if((((leftType & xmlSecTransformDataTypeBin) != 0) && 
        ((rightType & xmlSecTransformDataTypeBin) != 0)) || 
       (((leftType & xmlSecTransformDataTypeXml) != 0) && 
        ((rightType & xmlSecTransformDataTypeXml) != 0))) {
	
	left->next = right;
	right->prev = left;
	return(0);
    } 
    
    if(((leftType & xmlSecTransformDataTypeBin) != 0) && 
        ((rightType & xmlSecTransformDataTypeXml) != 0)) {
	    
	/* need to insert parser */
	middleId = xmlSecTransformXmlParserId;
    } else if(((leftType & xmlSecTransformDataTypeXml) != 0) && 
        ((rightType & xmlSecTransformDataTypeBin) != 0)) {
	
	/* need to insert c14n or special pre-base64 transform */
	if(xmlSecTransformCheckId(right, xmlSecTransformBase64Id)) {
	    middleId = xmlSecTransformRemoveXmlTagsC14NId;
	} else {
	    middleId = xmlSecTransformInclC14NId;
	}
    } else {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(left)),
		    xmlSecErrorsSafeString(xmlSecTransformGetName(right)),
		    XMLSEC_ERRORS_R_INVALID_TRANSFORM,
		    "leftType=%d;rightType=%d", 
		    leftType, rightType);
	return(-1);	
    }
    
    /* insert transform */
    middle = xmlSecTransformCreate(middleId);
    if(middle == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(left)),
		    "xmlSecTransformCreate",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    "transform=%s",
		    xmlSecErrorsSafeString(xmlSecTransformKlassGetName(middleId)));
	return(-1);
    }	
    left->next = middle;
    middle->prev = left;
    middle->next = right;
    right->prev = middle;
    return(0);
}


/**
 * xmlSecTransformAddAfter:
 * @curTransform: the pointer to current transform (may be NULL).
 * @newTransform: the pointer to new transform.
 * 
 * Adds @newTransform after the @curTransform.
 *
 * Returns pointer to the new transforms chain or NULL if an error occurs.
 */
xmlSecTransformPtr	
xmlSecTransformAddAfter(xmlSecTransformPtr curTransform, 
			xmlSecTransformPtr newTransform) {
    xmlSecAssert2(xmlSecTransformIsValid(newTransform), NULL);

    if(curTransform != NULL) {
	xmlSecAssert2(xmlSecTransformIsValid(curTransform), NULL);

	newTransform->prev = curTransform;
	newTransform->next = curTransform->next;
	curTransform->next = newTransform;
	if(newTransform->next != NULL) {
	    newTransform->next->prev = newTransform;
	}
    } else {
 	newTransform->next = newTransform->prev = NULL;
    }
    return(newTransform);
}

/**
 * xmlSecTransformAddBefore
 * @curTransform: the pointer to current transform (may be NULL).
 * @newTransform: the pointer to new transform.
 * 
 * Adds @newTransform before the @curTransform.
 *
 * Returns pointer to the new transforms chain or NULL if an error occurs.
 */
xmlSecTransformPtr	
xmlSecTransformAddBefore(xmlSecTransformPtr curTransform, 
			    xmlSecTransformPtr newTransform) {
    xmlSecAssert2(xmlSecTransformIsValid(newTransform), NULL);

    if(curTransform != NULL) {
	xmlSecAssert2(xmlSecTransformIsValid(curTransform), NULL);

	newTransform->next = curTransform;
	newTransform->prev = curTransform->prev;
	curTransform->prev = newTransform;
	if(newTransform->prev != NULL) {
	    newTransform->prev->next = newTransform;
	}
    } else {
	newTransform->next = newTransform->prev = NULL;
    }
    return(newTransform);
    
}

/**
 * xmlSecTransformRemove:
 * @transform: the pointer to #xmlSecTransform structure.
 *
 * Removes @transform from the chain. This method MUST be called by any 
 * bin transform id destructor.
 */
void
xmlSecTransformRemove(xmlSecTransformPtr transform) {
    xmlSecAssert(xmlSecTransformIsValid(transform));

    if(transform->next != NULL) {
	transform->next->prev = transform->prev;
    }
    if(transform->prev != NULL) {
	transform->prev->next = transform->next;
    }
    transform->next = transform->prev = NULL;
}


/************************************************************************
 *
 * Default callbacks, most of the transforms can use them
 *
 ************************************************************************/ 
xmlSecTransformDataType 
xmlSecTransformDefaultGetDataType(xmlSecTransformPtr transform, xmlSecTransformMode mode,
				  xmlSecTransformCtxPtr transformCtx) {
    xmlSecTransformDataType type = xmlSecTransformDataTypeUnknown;
    
    xmlSecAssert2(xmlSecTransformIsValid(transform), xmlSecTransformDataTypeUnknown);
    xmlSecAssert2(transformCtx != NULL, xmlSecTransformDataTypeUnknown);

    /* we'll try to guess the data type based on the handlers we have */
    switch(mode) {
	case xmlSecTransformModePush:
	    if(transform->id->pushBin != NULL) {
		type |= xmlSecTransformDataTypeBin;
	    } 
	    if(transform->id->pushXml != NULL) {
		type |= xmlSecTransformDataTypeXml;
	    } 
	    break;
	case xmlSecTransformModePop:
	    if(transform->id->popBin != NULL) {
		type |= xmlSecTransformDataTypeBin;
	    } 
	    if(transform->id->popXml != NULL) {
		type |= xmlSecTransformDataTypeXml;
	    } 
	    break;
	default:
	    xmlSecError(XMLSEC_ERRORS_HERE,
			xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
			NULL,
		        XMLSEC_ERRORS_R_INVALID_DATA,
			"mode=%d", mode);
	    return(xmlSecTransformDataTypeUnknown);
    }
    
    return(type);
}

int 
xmlSecTransformDefaultPushBin(xmlSecTransformPtr transform, const unsigned char* data,
			size_t dataSize, int final, xmlSecTransformCtxPtr transformCtx) {
    size_t inSize = 0;
    size_t outSize = 0;
    int finalData = 0;
    int ret;
    
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(transformCtx != NULL, -1);
    
    do {
        /* append data to input buffer */    
	if(dataSize > 0) {
	    size_t chunkSize;
	    
	    xmlSecAssert2(data != NULL, -1);

	    chunkSize = dataSize;
	    if(chunkSize > XMLSEC_TRANSFORM_BINARY_CHUNK) {
		chunkSize = XMLSEC_TRANSFORM_BINARY_CHUNK;
	    }
	    
	    ret = xmlSecBufferAppend(&(transform->inBuf), data, chunkSize);
	    if(ret < 0) {
		xmlSecError(XMLSEC_ERRORS_HERE,
			    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
			    "xmlSecBufferAppend",
			    XMLSEC_ERRORS_R_XMLSEC_FAILED,
			    "size=%d", chunkSize);
		return(-1);
	    }	

	    dataSize -= chunkSize;
	    data += chunkSize;
	}

	/* process data */
	inSize = xmlSecBufferGetSize(&(transform->inBuf));
	outSize = xmlSecBufferGetSize(&(transform->outBuf));
	finalData = (((dataSize == 0) && (final != 0)) ? 1 : 0);
	ret = xmlSecTransformExecute(transform, finalData, transformCtx);
	if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
			"xmlSecTransformExecute",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			"final=%d", final);
	    return(-1);
	}

	/* push data to the next transform */
	inSize = xmlSecBufferGetSize(&(transform->inBuf));
	outSize = xmlSecBufferGetSize(&(transform->outBuf));
	if(inSize > 0) {
	    finalData = 0;
	}

	/* we don't want to puch too much */
	if(outSize > XMLSEC_TRANSFORM_BINARY_CHUNK) {
	    outSize = XMLSEC_TRANSFORM_BINARY_CHUNK;
	    finalData = 0;
	}
	if((transform->next != NULL) && ((outSize > 0) || (finalData != 0))) {
	    ret = xmlSecTransformPushBin(transform->next, 
			    xmlSecBufferGetData(&(transform->outBuf)),
			    outSize,
			    finalData,
			    transformCtx);
	    if(ret < 0) {
		xmlSecError(XMLSEC_ERRORS_HERE,
			    xmlSecErrorsSafeString(xmlSecTransformGetName(transform->next)),
			    "xmlSecTransformPushBin",
			    XMLSEC_ERRORS_R_XMLSEC_FAILED,
			    "final=%d;outSize=%d", final,outSize);
		return(-1);
	    }
	}
	
	/* remove data anyway */
	if(outSize > 0) {
	    ret = xmlSecBufferRemoveHead(&(transform->outBuf), outSize);
	    if(ret < 0) {
		xmlSecError(XMLSEC_ERRORS_HERE,
			    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
			    "xmlSecBufferAppend",
			    XMLSEC_ERRORS_R_XMLSEC_FAILED,
			    "size=%d", outSize);
		return(-1);
	    }
	}
    } while((dataSize > 0) || (outSize > 0));
    
    return(0);
}

int 
xmlSecTransformDefaultPopBin(xmlSecTransformPtr transform, unsigned char* data,
			    size_t maxDataSize, size_t* dataSize, xmlSecTransformCtxPtr transformCtx) {
    size_t outSize;
    int final = 0;
    int ret;

    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(data != NULL, -1);
    xmlSecAssert2(dataSize != NULL, -1);
    xmlSecAssert2(transformCtx != NULL, -1);

    while((xmlSecBufferGetSize(&(transform->outBuf)) == 0) && (final == 0)) {
	/* read data from previous transform if exist */
	if(transform->prev != NULL) {    
    	    size_t inSize, chunkSize;

	    inSize = xmlSecBufferGetSize(&(transform->inBuf));
	    chunkSize = XMLSEC_TRANSFORM_BINARY_CHUNK;

	    /* ensure that we have space for at least one data chunk */
    	    ret = xmlSecBufferSetMaxSize(&(transform->inBuf), inSize + chunkSize);
    	    if(ret < 0) {
		xmlSecError(XMLSEC_ERRORS_HERE,
			    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
			    "xmlSecBufferSetMaxSize",
			    XMLSEC_ERRORS_R_XMLSEC_FAILED,
			    "size=%d", inSize + chunkSize);
		return(-1);
	    }	

	    /* get data from previous transform */
	    ret = xmlSecTransformPopBin(transform->prev, 
			    xmlSecBufferGetData(&(transform->inBuf)) + inSize,
			    chunkSize, &chunkSize, transformCtx);
	    if(ret < 0) {
		xmlSecError(XMLSEC_ERRORS_HERE,
			    xmlSecErrorsSafeString(xmlSecTransformGetName(transform->prev)),
			    "xmlSecTransformPopBin",
			    XMLSEC_ERRORS_R_XMLSEC_FAILED,
			    XMLSEC_ERRORS_NO_MESSAGE);
		return(-1);
	    }
	
	    /* adjust our size if needed */
	    if(chunkSize > 0) {
		ret = xmlSecBufferSetSize(&(transform->inBuf), inSize + chunkSize);
		if(ret < 0) {
		    xmlSecError(XMLSEC_ERRORS_HERE,
				xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
				"xmlSecBufferSetSize",
				XMLSEC_ERRORS_R_XMLSEC_FAILED,
				"size=%d", inSize + chunkSize);
		    return(-1);
	        }
		final = 0; /* the previous transform returned some data..*/
	    } else {
		final = 1; /* no data returned from previous transform, we are done */
	    }
	} else {
	    final = 1; /* no previous transform, we are "permanently final" */
	}	

	/* execute our transform */
    	ret = xmlSecTransformExecute(transform, final, transformCtx);
	if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
			"xmlSecTransformExecute",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			XMLSEC_ERRORS_NO_MESSAGE);
	    return(-1);
	}
    }
    
    /* copy result (if any) */
    outSize = xmlSecBufferGetSize(&(transform->outBuf)); 
    if(outSize > maxDataSize) {
	outSize = maxDataSize;
    }
    
    /* we don't want to put too much */
    if(outSize > XMLSEC_TRANSFORM_BINARY_CHUNK) {
	outSize = XMLSEC_TRANSFORM_BINARY_CHUNK;
    }
    if(outSize > 0) {
	xmlSecAssert2(xmlSecBufferGetData(&(transform->outBuf)), -1);
	
	memcpy(data, xmlSecBufferGetData(&(transform->outBuf)), outSize);

	ret = xmlSecBufferRemoveHead(&(transform->outBuf), outSize);
    	if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
			"xmlSecBufferRemoveHead",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			"size=%d", outSize);
	    return(-1);
	}	
    }
    
    /* set the result size */
    (*dataSize) = outSize;
    return(0);
}

int 
xmlSecTransformDefaultPushXml(xmlSecTransformPtr transform, xmlSecNodeSetPtr nodes, 
			    xmlSecTransformCtxPtr transformCtx) {
    int ret;

    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(transform->inNodes == NULL, -1);
    xmlSecAssert2(transform->outNodes == NULL, -1);
    xmlSecAssert2(transformCtx != NULL, -1);

    /* execute our transform */
    transform->inNodes = nodes;
    ret = xmlSecTransformExecute(transform, 1, transformCtx);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    "xmlSecTransformExecute",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }

    /* push result to the next transform (if exist) */
    if(transform->next != NULL) {
	ret = xmlSecTransformPushXml(transform->next, transform->outNodes, transformCtx);
	if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
			"xmlSecTransformPushXml",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			XMLSEC_ERRORS_NO_MESSAGE);
	    return(-1);
	}
    }        
    return(0);
}

int xmlSecTransformDefaultPopXml(xmlSecTransformPtr transform, xmlSecNodeSetPtr* nodes, 
			    xmlSecTransformCtxPtr transformCtx) {
    int ret;
    
    xmlSecAssert2(xmlSecTransformIsValid(transform), -1);
    xmlSecAssert2(transform->inNodes == NULL, -1);
    xmlSecAssert2(transform->outNodes == NULL, -1);
    xmlSecAssert2(transformCtx != NULL, -1);
    
    /* pop result from the prev transform (if exist) */
    if(transform->prev != NULL) {
	ret = xmlSecTransformPopXml(transform->prev, &(transform->inNodes), transformCtx);
	if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
			"xmlSecTransformPopXml",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			XMLSEC_ERRORS_NO_MESSAGE);
	    return(-1);
	}
    }        

    /* execute our transform */
    ret = xmlSecTransformExecute(transform, 1, transformCtx);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    "xmlSecTransformExecute",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }

    /* return result if requested */
    if(nodes != NULL) {
	(*nodes) = transform->outNodes;
    }
    
    return(0);
}

/***********************************************************************
 *
 * Transform Ids list
 *
 **********************************************************************/
static xmlSecPtrListKlass xmlSecTransformIdListKlass = {
    BAD_CAST "transform-ids-list",
    NULL, 							/* xmlSecPtrDuplicateItemMethod duplicateItem; */
    NULL,							/* xmlSecPtrDestroyItemMethod destroyItem; */
    NULL,							/* xmlSecPtrDebugDumpItemMethod debugDumpItem; */
    NULL,							/* xmlSecPtrDebugDumpItemMethod debugXmlDumpItem; */
};

xmlSecPtrListId 
xmlSecTransformIdListGetKlass(void) {
    return(&xmlSecTransformIdListKlass);
}

int 
xmlSecTransformIdListFind(xmlSecPtrListPtr list, xmlSecTransformId transformId) {
    size_t i, size;
    
    xmlSecAssert2(xmlSecPtrListCheckId(list, xmlSecTransformIdListId), -1);
    xmlSecAssert2(transformId != NULL, -1);
    
    size = xmlSecPtrListGetSize(list);
    for(i = 0; i < size; ++i) {
	if((xmlSecTransformId)xmlSecPtrListGetItem(list, i) == transformId) {
	    return(1);
	}
    }
    return(0);
}

xmlSecTransformId	
xmlSecTransformIdListFindByHref(xmlSecPtrListPtr list, const xmlChar* href,
			    xmlSecTransformUsage usage) {
    xmlSecTransformId transformId;
    size_t i, size;
    
    xmlSecAssert2(xmlSecPtrListCheckId(list, xmlSecTransformIdListId), xmlSecTransformIdUnknown);
    xmlSecAssert2(href != NULL, xmlSecTransformIdUnknown);
    
    size = xmlSecPtrListGetSize(list);
    for(i = 0; i < size; ++i) {
	transformId = (xmlSecTransformId)xmlSecPtrListGetItem(list, i);
	xmlSecAssert2(transformId != xmlSecTransformIdUnknown, xmlSecTransformIdUnknown);

	if(((usage & transformId->usage) != 0) && (transformId->href != NULL) && 
	   xmlStrEqual(href, transformId->href)) {
	   
	   return(transformId);	   
	}
    }
    return(xmlSecTransformIdUnknown);
}

xmlSecTransformId	
xmlSecTransformIdListFindByName(xmlSecPtrListPtr list, const xmlChar* name, 
			    xmlSecTransformUsage usage) {
    xmlSecTransformId transformId;
    size_t i, size;
    
    xmlSecAssert2(xmlSecPtrListCheckId(list, xmlSecTransformIdListId), xmlSecTransformIdUnknown);
    xmlSecAssert2(name != NULL, xmlSecTransformIdUnknown);

    size = xmlSecPtrListGetSize(list);
    for(i = 0; i < size; ++i) {
	transformId = (xmlSecTransformId)xmlSecPtrListGetItem(list, i);
	xmlSecAssert2(transformId != xmlSecTransformIdUnknown, xmlSecTransformIdUnknown);

	if(((usage & transformId->usage) != 0) && (transformId->name != NULL) &&
	   xmlStrEqual(name, BAD_CAST transformId->name)) {
	   
	   return(transformId);	   
	}
    }
    return(xmlSecTransformIdUnknown);
}

/************************************************************************
 *
 * IO buffers for transforms
 *
 ************************************************************************/ 
typedef struct _xmlSecTransformIOBuffer			xmlSecTransformIOBuffer,
							*xmlSecTransformIOBufferPtr;
typedef enum {
    xmlSecTransformIOBufferModeRead,
    xmlSecTransformIOBufferModeWrite
} xmlSecTransformIOBufferMode;

struct _xmlSecTransformIOBuffer {
    xmlSecTransformIOBufferMode		mode;
    xmlSecTransformPtr			transform;
    xmlSecTransformCtxPtr		transformCtx;
};

static xmlSecTransformIOBufferPtr xmlSecTransformIOBufferCreate	(xmlSecTransformIOBufferMode mode,
								 xmlSecTransformPtr transform,
								 xmlSecTransformCtxPtr transformCtx);
static void	xmlSecTransformIOBufferDestroy			(xmlSecTransformIOBufferPtr buffer);
static int	xmlSecTransformIOBufferRead			(xmlSecTransformIOBufferPtr buffer,
								 unsigned char *buf,
								 size_t size);		
static int	xmlSecTransformIOBufferWrite			(xmlSecTransformIOBufferPtr buffer,
								 const unsigned char *buf,
								 size_t size);		
static int	xmlSecTransformIOBufferClose			(xmlSecTransformIOBufferPtr buffer);


xmlOutputBufferPtr 
xmlSecTransformCreateOutputBuffer(xmlSecTransformPtr transform, xmlSecTransformCtxPtr transformCtx) {
    xmlSecTransformIOBufferPtr buffer; 
    xmlSecTransformDataType type;
    xmlOutputBufferPtr output;
    
    xmlSecAssert2(xmlSecTransformIsValid(transform), NULL);
    xmlSecAssert2(transformCtx != NULL, NULL);
    
    /* check that we have binary push method for this transform */
    type = xmlSecTransformDefaultGetDataType(transform, xmlSecTransformModePush, transformCtx);
    if((type & xmlSecTransformDataTypeBin) == 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    NULL,
		    XMLSEC_ERRORS_R_INVALID_TRANSFORM,
		    "push binary data not supported");
	return(NULL);
    }
    
    buffer = xmlSecTransformIOBufferCreate(xmlSecTransformIOBufferModeWrite, transform, transformCtx);
    if(buffer == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    "xmlSecTransformIOBufferCreate",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(NULL);
    }
    
    output = xmlOutputBufferCreateIO((xmlOutputWriteCallback)xmlSecTransformIOBufferWrite,
				     (xmlOutputCloseCallback)xmlSecTransformIOBufferClose,
				     buffer,
				     NULL); 
    if(output == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    "xmlOutputBufferCreateIO",
		    XMLSEC_ERRORS_R_XML_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	xmlSecTransformIOBufferDestroy(buffer);
	return(NULL);
    }
    
    return(output);
}

xmlParserInputBufferPtr 
xmlSecTransformCreateInputBuffer(xmlSecTransformPtr transform, xmlSecTransformCtxPtr transformCtx) {
    xmlSecTransformIOBufferPtr buffer;    
    xmlSecTransformDataType type;
    xmlParserInputBufferPtr input;
    
    xmlSecAssert2(xmlSecTransformIsValid(transform), NULL);
    xmlSecAssert2(transformCtx != NULL, NULL);

    /* check that we have binary pop method for this transform */
    type = xmlSecTransformDefaultGetDataType(transform, xmlSecTransformModePop, transformCtx);
    if((type & xmlSecTransformDataTypeBin) == 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    NULL,
		    XMLSEC_ERRORS_R_INVALID_TRANSFORM,
		    "pop binary data not supported");
	return(NULL);
    }    

    buffer = xmlSecTransformIOBufferCreate(xmlSecTransformIOBufferModeRead, transform, transformCtx);
    if(buffer == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    "xmlSecTransformIOBufferCreate",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(NULL);
    }
    
    input = xmlParserInputBufferCreateIO((xmlInputReadCallback)xmlSecTransformIOBufferRead,
				     (xmlInputCloseCallback)xmlSecTransformIOBufferClose,
				     buffer,
				     XML_CHAR_ENCODING_NONE); 
    if(input == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(transform)),
		    "xmlParserInputBufferCreateIO",
		    XMLSEC_ERRORS_R_XML_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	xmlSecTransformIOBufferDestroy(buffer);
	return(NULL);
    }
    
    return(input);
}

static xmlSecTransformIOBufferPtr 
xmlSecTransformIOBufferCreate(xmlSecTransformIOBufferMode mode, xmlSecTransformPtr transform,
			      xmlSecTransformCtxPtr transformCtx) {
    xmlSecTransformIOBufferPtr buffer;
    
    xmlSecAssert2(xmlSecTransformIsValid(transform), NULL);
    xmlSecAssert2(transformCtx != NULL, NULL);
    
    buffer = (xmlSecTransformIOBufferPtr)xmlMalloc(sizeof(xmlSecTransformIOBuffer));
    if(buffer == NULL) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    NULL,
		    "xmlMalloc",
		    XMLSEC_ERRORS_R_MALLOC_FAILED,
		    "size=%d", sizeof(xmlSecTransformIOBuffer)); 
	return(NULL);
    }
    memset(buffer, 0, sizeof(xmlSecTransformIOBuffer));
    
    buffer->mode = mode;
    buffer->transform = transform;
    buffer->transformCtx = transformCtx;
    
    return(buffer);
}

static void 
xmlSecTransformIOBufferDestroy(xmlSecTransformIOBufferPtr buffer) {
    xmlSecAssert(buffer != NULL);

    memset(buffer, 0, sizeof(xmlSecTransformIOBuffer));
    xmlFree(buffer);
}

static int 
xmlSecTransformIOBufferRead(xmlSecTransformIOBufferPtr buffer, 
			    unsigned char *buf, size_t size) {
    int ret;
    
    xmlSecAssert2(buffer != NULL, -1);
    xmlSecAssert2(buffer->mode == xmlSecTransformIOBufferModeRead, -1);
    xmlSecAssert2(xmlSecTransformIsValid(buffer->transform), -1);
    xmlSecAssert2(buffer->transformCtx != NULL, -1);
    xmlSecAssert2(buf != NULL, -1);
    
    ret = xmlSecTransformPopBin(buffer->transform, buf, size, &size, buffer->transformCtx);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(buffer->transform)),
		    "xmlSecTransformPopBin",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    return(size);
}

static int 
xmlSecTransformIOBufferWrite(xmlSecTransformIOBufferPtr buffer, 
			    const unsigned char *buf, size_t size) {
    int ret;
    
    xmlSecAssert2(buffer != NULL, -1);
    xmlSecAssert2(buffer->mode == xmlSecTransformIOBufferModeWrite, -1);
    xmlSecAssert2(xmlSecTransformIsValid(buffer->transform), -1);
    xmlSecAssert2(buffer->transformCtx != NULL, -1);
    xmlSecAssert2(buf != NULL, -1);

    ret = xmlSecTransformPushBin(buffer->transform, buf, size, 0, buffer->transformCtx);
    if(ret < 0) {
	xmlSecError(XMLSEC_ERRORS_HERE,
		    xmlSecErrorsSafeString(xmlSecTransformGetName(buffer->transform)),
		    "xmlSecTransformPushBin",
		    XMLSEC_ERRORS_R_XMLSEC_FAILED,
		    XMLSEC_ERRORS_NO_MESSAGE);
	return(-1);
    }
    return(size);
}

static int 
xmlSecTransformIOBufferClose(xmlSecTransformIOBufferPtr buffer) {
    int ret;
    
    xmlSecAssert2(buffer != NULL, -1);
    xmlSecAssert2(xmlSecTransformIsValid(buffer->transform), -1);
    xmlSecAssert2(buffer->transformCtx != NULL, -1);
    
    /* need to flush write buffer before destroing */
    if(buffer->mode == xmlSecTransformIOBufferModeWrite) {
        ret = xmlSecTransformPushBin(buffer->transform, NULL, 0, 1, buffer->transformCtx);
	if(ret < 0) {
	    xmlSecError(XMLSEC_ERRORS_HERE,
			xmlSecErrorsSafeString(xmlSecTransformGetName(buffer->transform)),
			"xmlSecTransformPushBin",
			XMLSEC_ERRORS_R_XMLSEC_FAILED,
			XMLSEC_ERRORS_NO_MESSAGE);
	    return(-1);
	}
    }
    
    xmlSecTransformIOBufferDestroy(buffer);
    return(0);
}
