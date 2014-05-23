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


package components.controller.userDataCommands
{
	import components.controller.UserDataCommand;
	import components.model.IntegraModel;
	import components.model.Project;

	public class SetShowSceneInTitlebar extends UserDataCommand
	{
		public function SetShowSceneInTitlebar( showSceneInTitlebar:Boolean )
		{
			super();
			
			_showSceneInTitlebar = showSceneInTitlebar;
		}
		
		
		public function get showSceneInTitlebar():Boolean { return _showSceneInTitlebar; }
		
		
		override public function initialize( model:IntegraModel ):Boolean
		{
			return( _showSceneInTitlebar != model.project.projectUserData.showSceneInTitlebar );
		}
		
		
		override public function generateInverse( model:IntegraModel ):void
		{
			pushInverseCommand( new SetShowSceneInTitlebar( model.project.projectUserData.showSceneInTitlebar ) );
		}
		
		
		override public function execute( model:IntegraModel ):void
		{
			var project:Project = model.project;
			
			project.projectUserData.showSceneInTitlebar = _showSceneInTitlebar;
		}


		public override function getObjectsWhoseUserDataIsAffected( model:IntegraModel, results:Vector.<int> ):void
		{
			results.push( model.project.id );	
		}

				
		private var _showSceneInTitlebar:Boolean;
	}
}