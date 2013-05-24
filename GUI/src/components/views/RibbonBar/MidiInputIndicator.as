/* Integra Live graphical user interface
 *
 * Copyright (C) 2009 Birmingham City University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA   02110-1301,
 * USA.
 */


package components.views.RibbonBar
{
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.events.TimerEvent;
	import flash.utils.Timer;
	
	import mx.controls.Label;
	import mx.core.ScrollPolicy;
	import mx.core.UIComponent;
	
	import components.controller.serverCommands.ReceiveMidiInput;
	import components.model.Info;
	import components.utils.FontSize;
	import components.views.IntegraView;
	import components.views.InfoView.InfoMarkupForViews;
	

	public class MidiInputIndicator extends IntegraView
	{
		public function MidiInputIndicator()
		{
			super();
			
			horizontalScrollPolicy = ScrollPolicy.OFF;
			verticalScrollPolicy = ScrollPolicy.OFF;

			_ccLabel.text = _ccLabelText;
			_noteLabel.text = _noteLabelText;
			_ccValue.text = _nothingLabelText;
			_noteValue.text = _nothingLabelText;
			
			setStyle( "color", 0x808080 );
			alpha = 0.5;
			
			addChild( _ccLabel );
			addChild( _ccValue );
			addChild( _noteLabel );
			addChild( _noteValue );        
			
			mask = _mask;
			addChild( _mask );
			
			addUpdateMethod( ReceiveMidiInput, onMidiInput );
			
			addEventListener( Event.RESIZE, onResize );
			
			_ccHideTimer.addEventListener( TimerEvent.TIMER_COMPLETE, onHideCC );
			_noteHideTimer.addEventListener( TimerEvent.TIMER_COMPLETE, onHideNote );
		}
		
		
		override public function getInfoToDisplay( event:MouseEvent ):Info
		{
			return InfoMarkupForViews.instance.getInfoForView( "MidiInputIndicator" );
		}
		
		
		public function get fullWidth():Number { return FontSize.getTextRowHeight( this ) * 3.5; }

		
		public function onMidiInput( command:ReceiveMidiInput ):void
		{
			if( command.midiID != model.project.midi.id ) return;
			
			switch( command.type )
			{
				case ReceiveMidiInput.CC:
					_ccLabel.text = _ccLabelText + String( command.index );
					_ccValue.text = String( command.value );
					_ccHideTimer.reset();
					_ccHideTimer.start();
					break;
				
				case ReceiveMidiInput.NOTE_ON:
					_noteValue.text = String( command.index );
					_noteHideTimer.reset();
					_noteHideTimer.start();
					break;
			}
		}
		

		override public function styleChanged( style:String ):void
		{
			if( !style || style == FontSize.STYLENAME )
			{
				var fontSize:Number = getStyle( FontSize.STYLENAME );
				fontSize *= _fontSizeProportion;
				
				_ccLabel.setStyle( FontSize.STYLENAME, fontSize ); 
				_ccValue.setStyle( FontSize.STYLENAME, fontSize );
				_noteLabel.setStyle( FontSize.STYLENAME, fontSize ); 
				_noteValue.setStyle( FontSize.STYLENAME, fontSize );
				
				height = FontSize.getTextRowHeight( this );
			}			
		}
		
		
		override protected function updateDisplayList( width:Number, height:Number ):void
        {
            super.updateDisplayList( width, height );

            graphics.clear();
			
			graphics.lineStyle( 1, 0x808080, 0.5 );
			graphics.moveTo( 0, height / 2 );
			graphics.lineTo( height, 0 );
			graphics.lineTo( fullWidth - 1, 0 );
			graphics.lineTo( fullWidth - 1, height - 1 );
			graphics.lineTo( height, height - 1 );
			graphics.lineTo( 0, height / 2 );
        }


        private function onResize( event:Event ):void
        {
			repositionControls();
        	invalidateDisplayList();
			
			_mask.graphics.clear();
			_mask.graphics.beginFill( 0 );
			_mask.graphics.drawRect( 0, 0, width, height );
        }
		
		
		private function repositionControls():void
		{
			var marginLeft:Number = height;
			var labelWidth:Number = height * 1.3;
			var labelHeight:Number = height * 0.5;
			var marginY:Number = -2; 
			_ccLabel.x = marginLeft;
			_ccLabel.y = marginY;
			_ccValue.x = marginLeft + labelWidth; 
			_ccValue.y = marginY; 
			_noteLabel.x = marginLeft;
			_noteLabel.y = labelHeight + marginY;
			_noteValue.x = marginLeft + labelWidth; 
			_noteValue.y = labelHeight + marginY; 
		}
		
		
		private function onHideCC( event:TimerEvent ):void
		{
			_ccLabel.text = _ccLabelText;
			_ccValue.text = _nothingLabelText;
		}

		
		private function onHideNote( event:TimerEvent ):void
		{
			_noteValue.text = _nothingLabelText;
		}
		
		
		private var _ccLabel:Label = new Label;
		private var _ccValue:Label = new Label;
		private var _noteLabel:Label = new Label;
		private var _noteValue:Label = new Label;
		
		private var _ccHideTimer:Timer = new Timer( _hideMsecs, 1 );
		private var _noteHideTimer:Timer = new Timer( _hideMsecs, 1 );
		
		private var _mask:UIComponent = new UIComponent;

		private static const _fontSizeProportion:Number = 0.9;
		
		private static const _ccLabelText:String = "cc";
		private static const _noteLabelText:String = "note";
		private static const _nothingLabelText:String = " - ";
		
		private static const _hideMsecs:Number = 5000;
	}
}