package com.jedox.etl.core.scriptapi;

import java.lang.annotation.Target;
import java.lang.annotation.Retention;
import java.lang.annotation.ElementType;
import java.lang.annotation.RetentionPolicy;

import com.jedox.etl.core.component.ITypes;

@Target(ElementType.PARAMETER)
@Retention(RetentionPolicy.RUNTIME)
public @interface Scanable {
	ITypes.Managers type();
}
