/** 
 * XMLSec library
 *
 * X509 support
 *
 * See Copyright for the status of this software.
 * 
 * Author: Aleksey Sanin <aleksey@aleksey.com>
 */
#ifndef __XMLSEC_X509_H__
#define __XMLSEC_X509_H__    

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */ 

#ifndef XMLSEC_NO_X509

#include <xmlsec/xmlsec.h>
#include <xmlsec/object.h>
#include <xmlsec/keys.h>

typedef struct _xmlSecX509MngrKlass		xmlSecX509MngrKlass,
						*xmlSecX509MngrKlassPtr;
typedef struct _xmlSecX509Mngr			xmlSecX509Mngr,
						*xmlSecX509MngrPtr;
typedef struct _xmlSecX509CtxKlass		xmlSecX509CtxKlass,
						*xmlSecX509CtxKlassPtr;
typedef struct _xmlSecX509Ctx			xmlSecX509Ctx,
						*xmlSecX509CtxPtr;


/***************************************************************************
 *
 * xmlSecKeyDataX509
 *
 **************************************************************************/
typedef enum {
    xmlSecKeyDataX509ObjTypeCert,
    xmlSecKeyDataX509ObjTypeVerifiedCert,
    xmlSecKeyDataX509ObjTypeTrustedCert,
    xmlSecKeyDataX509ObjTypeCrl
} xmlSecKeyDataX509ObjType;

typedef xmlSecKeyPtr	(*xmlSecKeyDataX509GetKeyMethod)	(xmlSecKeyDataPtr data,
								 xmlSecKeysMngrCtxPtr keysMngrCtx);
/**
 * xmlSecKeyDataX509FindCertMethod:
 * @data: the key data pointer
 * @mngr: the keys manager.
 * @subjectName: the subject name string.
 * @issuerName: the issuer name string.
 * @issuerSerial: the issuer serial.
 * @ski: the SKI string.
 *
 * Searches for matching certificate in the x509 data and keys manager.
 *
 * Returns the pointer to key that matches given criteria or NULL 
 * if an error occurs or certificate not found.
 */
typedef xmlSecKeyPtr	(*xmlSecKeyDataX509FindCertMethod)	(xmlSecKeyDataPtr data,
								 xmlSecKeysMngrCtxPtr keysMngrCtx,
								 xmlChar *subjectName,
								 xmlChar *issuerName,
								 xmlChar *issuerSerial,
								 xmlChar *ski);

typedef int		(*xmlSecKeyDataX509AddObjMethod)	(xmlSecKeyDataPtr data,
								 const unsigned char* buf,
								 size_t size,
								 xmlSecKeyDataX509ObjType type);
/**
 * xmlSecKeyDataX509GetObjMethod:
 * @pos: the object position (for cert objects, if the @pos is 0
 * then the cert containig the key MUST be returned).
 *
 * Returns 1 if the object returned, 0 if there are no more
 * objects of this type and a negative value if an error occurs.
 */
typedef int		(*xmlSecKeyDataX509GetObjMethod)	(xmlSecKeyDataPtr data,
								 unsigned char** buf,
								 size_t* size,
								 xmlSecKeyDataX509ObjType type,
								 size_t pos);


typedef struct _xmlSecKeyDataX509IdStruct	xmlSecKeyDataX509IdStruct,
						*xmlSecKeyDataX509Id;
struct _xmlSecKeyDataX509IdStruct {
    /* same as xmlSecDataId */
    xmlSecKeyDataType			type;
    const xmlChar*			childNodeName;
    const xmlChar*			childNodeNs;
    xmlSecKeyOrigin			origin; 
    
    xmlSecKeyDataCreateMethod		create;
    xmlSecKeyDataDestroyMethod		destroy;
    xmlSecKeyDataDuplicateMethod	duplicate;
    xmlSecKeyDataReadXmlMethod		read;
    xmlSecKeyDataWriteXmlMethod		write;
    xmlSecKeyDataReadBinaryMethod	readBin;
    xmlSecKeyDataWriteBinaryMethod	writeBin;

    /* new in xmlSecKeyDataX509Id */
    xmlSecKeyDataX509GetKeyMethod	getKey;
    xmlSecKeyDataX509FindCertMethod	findCert;
    xmlSecKeyDataX509AddObjMethod	addObj;
    xmlSecKeyDataX509GetObjMethod	getObj;
};


XMLSEC_EXPORT xmlSecKeyPtr	xmlSecKeyDataX509ReadXml	(xmlSecKeyDataId id,
								 xmlSecKeysMngrCtxPtr keysMngrCtx,
								 xmlNodePtr node);
XMLSEC_EXPORT int		xmlSecKeyDataX509WriteXml	(xmlSecKeyPtr key,
								 xmlSecKeysMngrCtxPtr keysMngrCtx,
								 xmlNodePtr parent);
XMLSEC_EXPORT xmlSecKeyPtr	xmlSecKeyDataX509ReadBinary	(xmlSecKeyDataId id,
								 xmlSecKeysMngrCtxPtr keysMngrCtx,
								 const unsigned char *buf,
								 size_t size);
XMLSEC_EXPORT int		xmlSecKeyDataX509WriteBinary	(xmlSecKeyPtr key,
						    		 xmlSecKeysMngrCtxPtr keysMngrCtx,
								 unsigned char **buf,
								 size_t *size);


XMLSEC_EXPORT xmlSecKeyPtr	xmlSecKeyDataX509GetKey		(xmlSecKeyDataPtr data,
								 xmlSecKeysMngrCtxPtr keysMngrCtx);
XMLSEC_EXPORT xmlSecKeyPtr	xmlSecKeyDataX509FindCert	(xmlSecKeyDataPtr data,
								 xmlSecKeysMngrCtxPtr keysMngrCtx,
								 xmlChar *subjectName,
								 xmlChar *issuerName,
								 xmlChar *issuerSerial,
								 xmlChar *ski);
XMLSEC_EXPORT int 		xmlSecKeyDataX509AddObj		(xmlSecKeyDataPtr data,
								 const unsigned char* buf,
								 size_t size,
								 xmlSecKeyDataX509ObjType type);
XMLSEC_EXPORT int		xmlSecKeyDataX509GetObj		(xmlSecKeyDataPtr data,
								 unsigned char** buf,
								 size_t* size,
								 xmlSecKeyDataX509ObjType type,
								 size_t pos);













#endif /* XMLSEC_NO_X509 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __XMLSEC_X509_H__ */

