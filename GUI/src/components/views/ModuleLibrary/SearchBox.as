package components.views.ModuleLibrary
{
	import flash.events.Event;
	import flash.events.FocusEvent;
	import flash.events.KeyboardEvent;
	import flash.events.MouseEvent;
	import flash.geom.Point;
	import flash.geom.Rectangle;
	
	import mx.containers.Canvas;
	import mx.controls.Button;
	import mx.controls.TextInput;
	
	import components.utils.FontSize;
	import components.utils.Utilities;
	import components.views.Skins.CloseButtonSkin;
	
	public class SearchBox extends Canvas
	{
		public function SearchBox()
		{
			super();
			
			_input.restrict = Utilities.printableCharacterRestrict;
			_input.setStyle( "top", _inputMargin );
			_input.setStyle( "bottom", _inputMargin );
			
			_input.setStyle( "color", '#808080' );

			_input.setStyle( "borderSkin", null );
			_input.setStyle( "focusAlpha", 0 );
			_input.setStyle( "backgroundAlpha", 0 );
			_input.addEventListener( Event.CHANGE, onChangeInput );
			_input.addEventListener( FocusEvent.FOCUS_IN, onFocusIn );
			_input.addEventListener( FocusEvent.FOCUS_OUT, onFocusOut );
			
			addChild( _input );
			
			_clearButton.visible = false;
			_clearButton.setStyle( "skin", CloseButtonSkin );
			_clearButton.setStyle( "color", 0x808080 );
			_clearButton.addEventListener( MouseEvent.CLICK, onClear );
			addChild( _clearButton );

			showEmptyPrompt = true;
			
			addEventListener( Event.RESIZE, onResize );
			
		}

		
		public function get searchText():String { return _showingEmptyPrompt ? "" : _input.text; }
		
		public function set filteredEverything( filteredEverything:Boolean ):void
		{
			if( filteredEverything != _filteredEverything )
			{
				_filteredEverything = filteredEverything;
				invalidateDisplayList();
			}
		}
		
		
		override protected function updateDisplayList( width:Number, height:Number):void
		{
			super.updateDisplayList( width, height );
			
			graphics.clear();
			
			//drag magnifying glass
			var magnifierColor:uint = _input.getStyle( "color" );
			
			var magnifierRect:Rectangle = new Rectangle( 0, 0, height, height );
			magnifierRect.inflate( -height / 4, -height / 4 );
			
			var circleRadius:Number = magnifierRect.width * 0.33;
			var circleCenter:Point = new Point( magnifierRect.right - circleRadius, magnifierRect.top + circleRadius );
			
			graphics.lineStyle( 2, magnifierColor, showAsFound ? 1 : 0.5 );
			graphics.drawCircle( circleCenter.x, circleCenter.y, circleRadius );
			
			var handleOffset:Number = circleRadius * Math.SQRT1_2;
			graphics.moveTo( circleCenter.x - handleOffset, circleCenter.y + handleOffset );
			graphics.lineTo( magnifierRect.left, magnifierRect.bottom );
		}
		
		
		private function onResize( event:Event ):void
		{
			_input.x = height + _inputMargin;
			_input.width = width - height * 2 - _inputMargin * 2;
			
			_clearButton.width = _clearButton.height = height * 0.5;
			var clearButtonMargin:Number = ( height - _clearButton.height ) / 2;
			
			_clearButton.x = width + clearButtonMargin - height;
			_clearButton.y = clearButtonMargin;
			
		}
		
		
		private function onChangeInput( event:Event ):void
		{
			invalidateDisplayList();

			dispatchEvent( new Event( SEARCH_CHANGE_EVENT ) );
			
			_clearButton.visible = ( searchText.length > 0 );
		}
		
		
		private function onClear( event:MouseEvent ):void
		{
			showEmptyPrompt = true;
			dispatchEvent( new Event( SEARCH_CHANGE_EVENT ) );
			
			_clearButton.visible = false;
		}
		
		
		private function get showAsFound():Boolean
		{
			return !_showingEmptyPrompt && ( searchText.length > 0 ) && !_filteredEverything;
		}
		
		
		private function onFocusIn( event:Event ):void
		{
			if( _showingEmptyPrompt )
			{
				showEmptyPrompt = false;
			}
		}

		
		private function onFocusOut( event:Event ):void
		{
			if( _input.length == 0 )
			{
				showEmptyPrompt = true;
			}
		}

		
		private function set showEmptyPrompt( showEmptyPrompt:Boolean ):void
		{
			_showingEmptyPrompt = showEmptyPrompt;
			
			if( showEmptyPrompt )
			{
				_input.alpha = 0.5;
				_input.text = _emptyPrompt;
			}
			else
			{
				_input.alpha = 1;
				_input.text = "";
			}
		}
		
		
		private var _showingEmptyPrompt:Boolean;
		
		private var _input:TextInput = new TextInput;
		private var _clearButton:Button = new Button;
		
		private var _filteredEverything:Boolean = false;
		
		private static const _inputMargin:Number = 3;
		private static const _emptyPrompt:String = "Search Modules...";
		
		
		static public const SEARCH_CHANGE_EVENT:String = "SearchChange"; 
	}
}