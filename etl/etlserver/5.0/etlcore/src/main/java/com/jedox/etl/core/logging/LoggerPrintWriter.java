/**
 * Redistribution and use of this software and associated documentation
 * ("Software"), with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * 1. Redistributions of source code must retain copyright
 *    statements and notices.  Redistributions must also contain a
 *    copy of this document.
 *
 * 2. Redistributions in binary form must reproduce the
 *    above copyright notice, this list of conditions and the
 *    following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. The name "Exolab" must not be used to endorse or promote
 *    products derived from this Software without prior written
 *    permission of Intalio.  For written permission,
 *    please contact info@exolab.org.
 *
 * 4. Products derived from this Software may not be called "Exolab"
 *    nor may "Exolab" appear in their names without prior written
 *    permission of Intalio. Exolab is a registered
 *    trademark of Intalio.
 *
 * 5. Due credit should be given to the Exolab Project
 *    (http://www.exolab.org/).
 *
 * THIS SOFTWARE IS PROVIDED BY INTALIO AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * INTALIO OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright 1999-2001 (C) Intalio Inc. All Rights Reserved.
 *
 * Contributions by MetaBoss team are Copyright (c) 2003-2004, Softaris Pty. Ltd. All Rights Reserved.
 *
 * $Id: LoggerPrintWriter.java,v 1.1 2004/05/06 06:04:41 metaboss Exp $
 */

package com.jedox.etl.core.logging;

import java.io.PrintWriter;
import java.io.StringWriter;
import org.apache.log4j.Category;
import org.apache.log4j.Level;


/**
 * Print writer using a Log4J category. Connectors and other J2EE service
 * providers require a <tt>PrintWriter</tt> in order to log messages.
 * This object provides an implementation of <tt>PrintWriter</tt> that
 * logs the message to an underlying Log4J category.
 *
 * @author <a href="arkin@intalio.com">Assaf Arkin</a>
 * @version $Revision: 1.1 $ $Date: 2004/05/06 06:04:41 $
 */
public final class LoggerPrintWriter
    extends PrintWriter
{


    /**
     * The Log4J category to which messages are sent.
     */
    private final Category      _category;


    /**
     * The Log4J priority or all logged messsages.
     */
    private final Level      _priority;


    /**
     * The string buffer use to accumulate message parts until
     * a line is complete and flushed.
     */
    private final StringBuffer  _buffer;


    private int                 _lineNumber = -1;


    private static final int    LINE_NO_LENGTH = 3;


    /**
     * Constructs a new print writer using the underlying Log4J category.
     *
     * @param category A Log4J category to use for logging
     * @param priority A Log4J priority, or null for the default
     */
    public LoggerPrintWriter( Category category, Level priority )
    {
        this( category, priority, false );
    }


    /**
     * Constructs a new print writer using the underlying Log4J category.
     *
     * @param category A Log4J category to use for logging
     * @param priority A Log4J priority, or null for the default
     * @param lineNumbers True if line numbers should be printed
     */
    public LoggerPrintWriter( Category category, Level priority, boolean lineNumbers )
    {
        super( new StringWriter() );
        if ( category == null )
            throw new IllegalArgumentException( "Argument category is null" );
        _category = category;
        if ( priority == null )
            _priority = Level.INFO;
        else
            _priority = priority;
        _buffer = new StringBuffer();
        if ( lineNumbers ) {
            _lineNumber = 0;
            addLineNumber();
        } else
            _lineNumber = -1;
    }


    public void println()
    {
        flushLine();
    }


    public void println( boolean value )
    {
        _buffer.append( value );
        flushLine();
    }


    public void print( boolean value )
    {
        _buffer.append( value );
    }


    public void println( char value )
    {
        if ( value == '\n' )
            flushLine();
        else
            _buffer.append( value );
        flushLine();
    }


    public void print( char value )
    {
        if ( value == '\n' )
            flushLine();
        else
            _buffer.append( value );
    }


    public void println( int value )
    {
        _buffer.append( value );
        flushLine();
    }


    public void print( int value )
    {
        _buffer.append( value );
    }


    public void println( long value )
    {
        _buffer.append( value );
        flushLine();
    }


    public void print( long value )
    {
        _buffer.append( value );
    }


    public void println( float value )
    {
        _buffer.append( value );
        flushLine();
    }


    public void print( float value )
    {
        _buffer.append( value );
    }


    public void println( double value )
    {
        _buffer.append( value );
        flushLine();
    }


    public void print( double value )
    {
        _buffer.append( value );
    }


    public void println( char[] value )
    {
        char ch;
        int  length;

        if ( value != null ) {
            length = value.length;
            for ( int i = 0 ; i < length ; ++i ) {
                ch = value[ i ];
                if ( ch == '\n' )
                    flushLine();
                else
                    _buffer.append( ch );
            }
        }
        flushLine();
    }


    public void print( char[] value )
    {
        char ch;
        int  length;

        if ( value != null ) {
            length = value.length;
            for ( int i = 0 ; i < length ; ++i ) {
                ch = value[ i ];
                if ( ch == '\n' )
                    flushLine();
                else
                    _buffer.append( ch );
            }
        }
    }


    public void println( String value )
    {
        char ch;
        int  length;

        if ( value == null )
            _buffer.append( "null" );
        else {
            length = value.length();
            for ( int i = 0 ; i < length ; ++i ) {
                ch = value.charAt( i );
                if ( ch == '\n' )
                    flushLine();
                else
                    _buffer.append( ch );
            }
        }
        flushLine();
    }


    public void print( String value )
    {
        char ch;
        int  length;

        if ( value == null )
            _buffer.append( "null" );
        else {
            length = value.length();
            for ( int i = 0 ; i < length ; ++i ) {
                ch = value.charAt( i );
                if ( ch == '\n' )
                    flushLine();
                else
                    _buffer.append( ch );
            }
        }
    }


    public void println( Object value )
    {
        if ( value == null )
            _buffer.append( "null" );
        else
            _buffer.append( value.toString() );
        flushLine();
    }


    public void print( Object value )
    {
        if ( value == null )
            _buffer.append( "null" );
        else
            _buffer.append( value.toString() );
    }


    private void flushLine()
    {
        _category.log( _priority, _buffer.toString() );
        _buffer.setLength( 0 );
        if ( _lineNumber >= 0 )
            addLineNumber();
    }


    private void addLineNumber()
    {
        String string;
        int    length;
        int    start = 0;

        ++ _lineNumber;
        string = String.valueOf( _lineNumber );
        length = string.length();
        if ( length >= LINE_NO_LENGTH ) {
            length = LINE_NO_LENGTH;
            start = string.length() - length;
        } else {
            for ( int i = 0 ; i < LINE_NO_LENGTH - length ; ++i )
                _buffer.append( ' ' );
        }
        for ( int i = 0 ; i < length ; ++i )
            _buffer.append( string.charAt( i + start ) );
        _buffer.append( ':' ).append( ' ' );
    }


}
