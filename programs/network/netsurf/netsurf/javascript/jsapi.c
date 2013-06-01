/*
 * Copyright 2012 Vincent Sanders <vince@netsurf-browser.org>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "javascript/jsapi.h"
#include "render/html_internal.h"
#include "content/content.h"
#include "javascript/content.h"
#include "javascript/js.h"

#include "utils/log.h"

#include "window.h"
#include "event.h"

static JSRuntime *rt; /* global runtime */

void js_initialise(void)
{
	/* Create a JS runtime. */

#if JS_VERSION >= 180
	JS_SetCStringsAreUTF8(); /* we prefer our runtime to be utf-8 */
#endif

	rt = JS_NewRuntime(8L * 1024L * 1024L);
	JSLOG("New runtime handle %p", rt);

	if (rt != NULL) {
		/* register script content handler */
		javascript_init();
	}
}

void js_finalise(void)
{
	if (rt != NULL) {
		JSLOG("destroying runtime handle %p", rt);
		JS_DestroyRuntime(rt);
	}
	JS_ShutDown();
}

/* The error reporter callback. */
static void js_reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
	JSLOG("%s:%u:%s",
	      report->filename ? report->filename : "<no filename>",
	      (unsigned int) report->lineno,
	      message);
}

jscontext *js_newcontext(void)
{
	JSContext *cx;

	if (rt == NULL) {
		return NULL;
	}

	cx = JS_NewContext(rt, 8192);
	if (cx == NULL) {
		return NULL;
	}
	JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_JIT );
	JS_SetVersion(cx, JSVERSION_LATEST);
	JS_SetErrorReporter(cx, js_reportError);

	/*JS_SetGCZeal(cx, 2); */

	JSLOG("New Context %p", cx);

	return (jscontext *)cx;
}

void js_destroycontext(jscontext *ctx)
{
	JSContext *cx = (JSContext *)ctx;
	if (cx != NULL) {
		JSLOG("Destroying Context %p", cx);
		JS_DestroyContext(cx);
	}
}


/** Create new compartment to run scripts within
 *
 * This performs the following actions
 * 1. constructs a new global object by initialising a window class
 * 2. Instantiate the global a window object
 */
jsobject *js_newcompartment(jscontext *ctx, void *win_priv, void *doc_priv)
{
	JSContext *cx = (JSContext *)ctx;
	JSObject *window_proto;
	JSObject *window;

	if (cx == NULL) {
		return NULL;
	}

	window_proto = jsapi_InitClass_Window(cx, NULL);
	if (window_proto == NULL) {
		JSLOG("Unable to initialise window class");
		return NULL;
	}

	window = jsapi_new_Window(cx, window_proto, NULL, win_priv, doc_priv);

	return (jsobject *)window;
}

bool js_exec(jscontext *ctx, const char *txt, size_t txtlen)
{
	JSContext *cx = (JSContext *)ctx;
	jsval rval;

	/* JSLOG("%p \"%s\"",cx ,txt); */

	if (ctx == NULL) {
		return false;
	}

	if (txt == NULL) {
		return false;
	}

	if (txtlen == 0) {
		return false;
	}

	if (JS_EvaluateScript(cx,
			      JS_GetGlobalObject(cx),
			      txt, txtlen,
			      "<head>", 0, &rval) == JS_TRUE) {

		return true;
	}

	return false;
}

dom_exception _dom_event_create(dom_document *doc, dom_event **evt);
#define dom_event_create(d, e) _dom_event_create((dom_document *)(d), (dom_event **) (e))

bool js_fire_event(jscontext *ctx, const char *type, dom_document *doc, dom_node *target)
{
	JSContext *cx = (JSContext *)ctx;
	dom_node *node = target;
	JSObject *jsevent;
	jsval rval;
	jsval argv[1];
	JSBool ret = JS_TRUE;
	dom_exception exc;
	dom_event *event;
	dom_string *type_dom;

	if (cx == NULL) {
		return false;
	}

	if (node == NULL) {
		/* deliver manufactured event to window */
		JSLOG("Dispatching event %s at window", type);

		/* create and initialise and event object */
		exc = dom_string_create((unsigned char*)type,
					strlen(type),
					&type_dom);
		if (exc != DOM_NO_ERR) {
			return false;
		}

		exc = dom_event_create(doc, &event);
		if (exc != DOM_NO_ERR) {
			return false;
		}

		exc = dom_event_init(event, type_dom, false, false);
		dom_string_unref(type_dom);
		if (exc != DOM_NO_ERR) {
			return false;
		}

		jsevent = jsapi_new_Event(cx, NULL, NULL, event);
		if (jsevent == NULL) {
			return false;
		}

		/* dispatch event at the window object */
		argv[0] = OBJECT_TO_JSVAL(jsevent);

		ret = JS_CallFunctionName(cx,
					  JS_GetGlobalObject(cx),
					  "dispatchEvent",
					  1,
					  argv,
					  &rval);
	} else {
		JSLOG("Dispatching event %s at %p", type, node);

		/* create and initialise and event object */
		exc = dom_string_create((unsigned char*)type,
					strlen(type),
					&type_dom);
		if (exc != DOM_NO_ERR) {
			return false;
		}

		exc = dom_event_create(doc, &event);
		if (exc != DOM_NO_ERR) {
			return false;
		}

		exc = dom_event_init(event, type_dom, true, true);
		dom_string_unref(type_dom);
		if (exc != DOM_NO_ERR) {
			return false;
		}

		dom_event_target_dispatch_event(node, event, &ret);

	}

	if (ret == JS_TRUE) {
		return true;
	}
	return false;
}

struct js_dom_event_private {
	JSContext *cx; /* javascript context */
	jsval funcval; /* javascript function to call */
	struct dom_node *node; /* dom node event listening on */
	dom_string *type; /* event type */
	dom_event_listener *listener; /* the listener containing this */
};

static void
js_dom_event_listener(struct dom_event *event, void *pw)
{
	struct js_dom_event_private *private = pw;
	jsval event_argv[1];
	jsval event_rval;
	JSObject *jsevent;

	JSLOG("WOOT dom event with %p", private);

	if (!JSVAL_IS_VOID(private->funcval)) {
		jsevent = jsapi_new_Event(private->cx, NULL, NULL, event);
		if (jsevent != NULL) {

		/* dispatch event at the window object */
		event_argv[0] = OBJECT_TO_JSVAL(jsevent);

		JS_CallFunctionValue(private->cx,
				     NULL,
				     private->funcval,
				     1,
				     event_argv,
				     &event_rval);
		}
	}
}

/* add a listener to a dom node
 *
 * 1. Create a dom_event_listener From a handle_event function pointer
 *    and a private word In a document context
 *
 * 2. Register for your events on a target (dom nodes are targets)
 *    dom_event_target_add_event_listener(node, evt_name, listener,
 *    capture_or_not)
 *
 */

bool
js_dom_event_add_listener(jscontext *ctx,
			  struct dom_document *document,
			  struct dom_node *node,
			  struct dom_string *event_type_dom,
			  void *js_funcval)
{
	JSContext *cx = (JSContext *)ctx;
	dom_exception exc;
	struct js_dom_event_private *private;

	private = malloc(sizeof(struct js_dom_event_private));
	if (private == NULL) {
		return false;
	}

	exc = dom_event_listener_create(document,
					js_dom_event_listener,
					private,
					&private->listener);
	if (exc != DOM_NO_ERR) {
		return false;
	}

	private->cx = cx;
	private->funcval = *(jsval *)js_funcval;
	private->node = node;
	private->type = event_type_dom;

	JSLOG("adding %p to listener", private);

	JSAPI_ADD_VALUE_ROOT(cx, &private->funcval);
	exc = dom_event_target_add_event_listener(private->node,
						  private->type,
						  private->listener,
						  true);
	if (exc != DOM_NO_ERR) {
		JSLOG("failed to add listener");
		JSAPI_REMOVE_VALUE_ROOT(cx, &private->funcval);
	}

	return true;
}
