function init()
{
	nbl=0;
	nbc=0;
	timer=0;
	logical_date=0;
	nb_grains_case=[0,0,0,0,0]; // nb_grains_case[i] contient le nombre de cases à i grains, i=4 pour >=4 grains
	
	table=[];
	elements_effondrables=[];
	
	etat="attente";
	
	couleurs=["white", "rgb(124,151,255)", "blue", "rgb(0,0,155)","black"];
	
	document.getElementById('menu1').style.display="block";
	document.getElementById('menu2').style.display="none";
	document.getElementById('menu3').style.display="none";
	document.getElementById('menu4').style.display="none";
	document.getElementById('menu5').style.display="none";
	document.getElementById('menu6').style.display="none";
	document.getElementById('menu7').style.display="none";
	document.getElementById('alerts').innerHTML="Veuillez entrer les dimensions de la grille.";
	document.getElementById('content').innerHTML="";
	
}

function getcouleur(i)
{
	return (i<4)?couleurs[i]:couleurs[4];
}

function setcouleur(i,j)
{
	if(table.length > 0)
	{
		var nbgrains=table[i][j];
	}
	
	else
	{
		var nbgrains=document.getElementById('td'+i+'_'+j).textContent;
	}
	
	document.getElementById('td'+i+'_'+j).style.backgroundColor=getcouleur(nbgrains);
	if(nbgrains>0)
	{
		document.getElementById('td'+i+'_'+j).style.color=nbgrains>3?"white":"black";
	}
	
	else
	{
		if(table.length == 0) {document.getElementById('td'+i+'_'+j).style.color="black";}
		document.getElementById('td'+i+'_'+j).style.backgroundColor="white";
	}
}

function menu1()
{
	if (parseInt(document.getElementById('nbc').value)>0 && parseInt(document.getElementById('nbl').value)>0)
	{
		document.getElementById('menu1').style.display="none";
		document.getElementById('menu2').style.display="block";
		document.getElementById('menu3').style.display="block";
		document.getElementById('menu4').style.display="block";
		document.getElementById('menu5').style.display="block";
		document.getElementById('alerts').innerHTML="<span style=\"color:green\">La grille est créée.</span>";

		nbl=parseInt(document.getElementById('nbl').value);
		nbc = parseInt(document.getElementById('nbc').value);
		
		var tableauhtml = ""; // c'est sale comme méthode...
		
		tableauhtml+="<table>";
		
		for(var i=0;i<nbl;i++)
		{
			tableauhtml+="<tr>";
			
			for(var j=0;j<nbc;j++)
			{
				tableauhtml+='<td value="(' + i + ', '+j+')" id="td'+i+'_'+j+'" onmouseover="infocase('+i+', '+j+')" onclick="actioncase('+i+', '+j+')">0</td>';	
			}
			
			tableauhtml+="</tr>";

		}
		
		tableauhtml+="</table>";
			
		document.getElementById('content').innerHTML=tableauhtml;
		style_cases();
		
		
		
	}
	
	else
	{
		document.getElementById('alerts').innerHTML='<span style="color:red;">Les dimensions ne vont pas...</span>';
	}
}

function actioncase(i,j)
{
	if(document.getElementById('menu2').style.display != "none")
	{
		changecase(i,j);	
	}	
	
	else
	{
		if(table[i][j]>=4)
		{
			var nb_effondrables = elements_effondrables.length;
			var case_choisie=0;
			
			for(case_choisie=0;case_choisie<nb_effondrables;case_choisie++)
			{
				var couple = elements_effondrables[case_choisie];
				if(i==couple[0] && j==couple[1])
				{
					break;	
				}
			}
		
			var intermediaire=elements_effondrables[case_choisie];
			elements_effondrables[case_choisie]=elements_effondrables[nb_effondrables -1];
			elements_effondrables[nb_effondrables -1]=intermediaire;	
		}
		ecroule(i,j);	
	}
}

function parserInt(a)
{
	if(a=="" || typeof(a) == "undefined")
	{
		return 0;	
	}	
	
	else
	{
		return parseInt(a);	
	}
}

function ecroule(i,j,option)
{
	
	var val=table[i][j];
	
	
	if(val>=4)
	{	

		if(val<8) elements_effondrables.pop();
		
		if(i-1 >= 0){
			table[i-1][j]++;
			if(table[i-1][j]==4) elements_effondrables.push([i-1,j]);
		}
		if(i+1 < nbl){
			table[i+1][j]++;
			if(table[i+1][j]==4) elements_effondrables.push([i+1,j]);
		}
		if(j-1 >= 0){
			table[i][j-1]++;
			if(table[i][j-1]==4) elements_effondrables.push([i,j-1]);
		}
		if(j+1 < nbc){
			table[i][j+1]++;
			if(table[i][j+1]==4) elements_effondrables.push([i,j+1]);
		}
		
		logical_date++;
		
		table[i][j]-=4;
		
		if (typeof(option) == 'undefined')
		{
			if(i-1 >= 0) {
				document.getElementById('td'+(i-1)+'_'+j).textContent=table[i-1][j];
				setcouleur(i-1,j);}
			if(i+1 < nbl) {
				document.getElementById('td'+(i+1)+'_'+j).textContent=table[i+1][j];
				setcouleur(i+1,j);}
			if(j-1 >= 0) {
				document.getElementById('td'+i+'_'+(j-1)).textContent=table[i][j-1];
				setcouleur(i,j-1);}
			if(j+1 < nbc){
				document.getElementById('td'+i+'_'+(j+1)).textContent=table[i][j+1];
				setcouleur(i,j+1);}	
		
			if(val=='4')
			{
				document.getElementById('td'+i+'_'+j).textContent='';
			}
		
			else
			{
				document.getElementById('td'+i+'_'+j).textContent=table[i][j];
			}
			
			document.getElementById('date').textContent=logical_date;
			setcouleur(i,j);
			enveloppeconvexe();
		
		}
		
				
	}
	
	else
	{
		var message='<span style="color:red">Cette case n\'est pas effondrable</span>';
		document.getElementById('alerts').innerHTML=message;	
	}
	
}

function menu2()
{
	var i=document.getElementById('vall').value;
	var j=document.getElementById('valc').value;
	var val=document.getElementById('valcase').value;
	
	document.getElementById('td'+i+'_'+j).textContent=parseInt(val);
	setcouleur(i,j);
}

function infocase(i,j)
{
	var valeur = document.getElementById('td'+i+'_'+j).textContent;
	var message='<span class="espace">ligne : ' + i + '</span><span class="espace">colonne : ' + j + '</span>Valeur : ' + parserInt(valeur);
	
	
	document.getElementById('alerts').innerHTML=message;	
}

function changecase(i,j)
{
	var valeur = parseInt(document.getElementById('td'+i+'_'+j).textContent);
	
	if(etat=="attente")
	{
		document.getElementById('vall').value=i;
		document.getElementById('valc').value=j;
		document.getElementById('valcase').value=valeur;
	
		document.getElementById('valcase').focus();
	}
	
	else if(etat=="etape1_r" || etat=="etape1")
	{
		document.getElementById('abx1').value=i;
		document.getElementById('aby1').value=j;
	
		if(etat=="etape1_r"){
			document.getElementById('rayon').focus();
		}
		
		else{
			document.getElementById('aby1').focus();
		}
	}
	
	else if(etat=="etape2")
	{
		document.getElementById('abx2').value=i;
		document.getElementById('aby2').value=j;
		document.getElementById('aby2').focus();
	}

}

function clear_info()
{
	document.getElementById('alerts').innerHTML="";
}

function getsomme()
{
	var somme=0;	
	for(var i=0;i<nbl;i++)
		{
						
			for(var j=0;j<nbc;j++)
			{
				somme+=table[i][j];
			}
		}
		
		return somme;
}

function style_cases()
{
	var largeur = parseInt(document.getElementById('largeur').value)+ 'px';
	var longueur = parseInt(document.getElementById('longueur').value) + 'px';
	var taille = parseInt(document.getElementById('taille_police').value) + 'px';
	document.getElementById('content').style.fontSize=taille;
	
	for(var i=0;i<nbl;i++)
		{
						
			for(var j=0;j<nbc;j++)
			{
				document.getElementById('td'+i+'_'+j).style.width=largeur;
				document.getElementById('td'+i+'_'+j).style.minWidth=largeur;
				document.getElementById('td'+i+'_'+j).style.height=longueur;
			}			
			

		}
		
}

function alea1()
{
	etat="etape1";
	var choix = document.getElementById('pattern').value;
	if(choix=="losange" || choix=="cercle")
	{
		etat="etape1_r";
		document.getElementById('formalea1').style.display="none";
		document.getElementById('pabx2').style.display="none";
		document.getElementById('paby2').style.display="none";
		document.getElementById('pabx1').style.display="block";
		document.getElementById('paby1').style.display="block";

		document.getElementById('pr').style.display="block";

		document.getElementById('indication').innerHTML="Centre et rayon ?";
		document.getElementById('formalea2').style.display="block";
	}
	
	else if(choix=="partout")
	{
		alea();	
	}
	
	else
	{
		document.getElementById('formalea1').style.display="none";
		document.getElementById('pr').style.display="none";
		document.getElementById('pabx2').style.display="none";
		document.getElementById('paby2').style.display="none";
		document.getElementById('pabx1').style.display="block";
		document.getElementById('paby1').style.display="block";
		document.getElementById('indication').innerHTML="Première case ?";
		document.getElementById('formalea2').style.display="block";
		document.getElementById('formalea2').onsubmit=function(){alea2();return false;};	
	}
}

function alea2()
{
	etat="etape2";
	var i_0=parseInt(document.getElementById('abx1').value);
	var j_0=parseInt(document.getElementById('aby1').value);
	document.getElementById('td'+i_0+'_'+j_0).style.backgroundColor="orange";
	document.getElementById('pabx1').style.display="none";
	document.getElementById('paby1').style.display="none";
	document.getElementById('pabx2').style.display="block";
	document.getElementById('paby2').style.display="block";
	
	document.getElementById('indication').innerHTML="Deuxième case ?";
	document.getElementById('formalea2').onsubmit=function(){alea();return false;};
}

function alea()
{
	etat="attente";
	document.getElementById('formalea2').style.display="none";
	document.getElementById('formalea1').style.display="block";
	
	var	maxi=parseInt(document.getElementById('randmax').value);
	var mini = parseInt(document.getElementById('randmin').value);
	var choix = document.getElementById('pattern').value;
	
	
	if(choix=="losange" || choix=="cercle")
	{
		var i_0=parseInt(document.getElementById('abx1').value);
		var j_0 = parseInt(document.getElementById('aby1').value);;
		var r=parseInt(document.getElementById('rayon').value);;
		
		
		if(i_0<0 || j_0<0 || i_0>=nbl || j_0 >=nbc)
		{
			alert("données non cohérentes entre elles ou avec la grille !");
			return 0;
		}
		else
		{
			if(choix=="losange")
			{
				for(var i=0;i<=r;i++)
				{
					for(var j=0;j<=r-i;j++)
					{
						if(i_0+i<nbl && j_0+j < nbc)
						{
							document.getElementById('td'+(i_0+i)+'_'+(j_0+j)).textContent=Math.floor(Math.random() * (maxi - mini + 1)) + mini;
							setcouleur(i_0+i,j_0+j);

						}
						
						if(i_0-i>=0 && j_0+j < nbc)
						{
							document.getElementById('td'+(i_0-i)+'_'+(j_0+j)).textContent=Math.floor(Math.random() * (maxi - mini + 1)) + mini;
							setcouleur(i_0-i,j_0+j);	
						}
						
						if(i_0+i<nbl && j_0-j >= 0)
						{
							document.getElementById('td'+(i_0+i)+'_'+(j_0-j)).textContent=Math.floor(Math.random() * (maxi - mini + 1)) + mini;
							setcouleur(i_0+i,j_0-j);
						}
						
						if(i_0-i>=0 && j_0-j >=0)
						{
							document.getElementById('td'+(i_0-i)+'_'+(j_0-j)).textContent=Math.floor(Math.random() * (maxi - mini + 1)) + mini;
							setcouleur(i_0-i,j_0-j);
						}
					}
				}
			}
			
			else // choix==cercle
			{
				for(var i=0;i<=r;i++)
				{
					for(var j=0;j<=r;j++)
					{
						if(Math.floor(Math.sqrt(i*i+j*j))<=r)
						{
							if(i_0+i<nbl && j_0+j < nbc)
							{
								document.getElementById('td'+(i_0+i)+'_'+(j_0+j)).textContent=Math.floor(Math.random() * (maxi - mini + 1)) + mini;
								setcouleur(i_0+i,j_0+j);

							}
						
							if(i_0-i>=0 && j_0+j < nbc)
							{
								document.getElementById('td'+(i_0-i)+'_'+(j_0+j)).textContent=Math.floor(Math.random() * (maxi - mini + 1)) + mini;	
								setcouleur(i_0-i,j_0+j);
							}
						
							if(i_0+i<nbl && j_0-j >= 0)
							{
								document.getElementById('td'+(i_0+i)+'_'+(j_0-j)).textContent=Math.floor(Math.random() * (maxi - mini + 1)) + mini;
								setcouleur(i_0+i,j_0-j);
							}
						
							if(i_0-i>=0 && j_0-j >=0)
							{
								document.getElementById('td'+(i_0-i)+'_'+(j_0-j)).textContent=Math.floor(Math.random() * (maxi - mini + 1)) + mini;
								setcouleur(i_0-i,j_0-j);
							}
						}
					}
				}
			}
		}
	}
	
	else
	{
		var i_0=0;
		var i_1 = nbl-1;
		var j_0=0;
		var j_1 = nbc-1;
		
		if(choix=="rectangle")
		{
			var i_2=parseInt(document.getElementById('abx1').value);
			var i_3=parseInt(document.getElementById('abx2').value);
		 	var j_2=parseInt(document.getElementById('aby1').value);
			var j_3=parseInt(document.getElementById('aby2').value);
			
			i_0=Math.min(i_2,i_3);
			i_1=Math.max(i_2,i_3);
			j_0=Math.min(j_2,j_3);
			j_1=Math.max(j_2,j_3);
		
			if(i_0>i_1 || j_0>j_1 || i_0<0 || j_0<0 || i_1>=nbl || j_1 >=nbc)
			{
				alert("données non cohérentes entre elles ou avec la grille !");
				return 0;
			}
		}
		
		for(var i=i_0;i<=i_1;i++)
		{
						
			for(var j=j_0;j<=j_1;j++)
			{
				document.getElementById('td'+i+'_'+j).textContent=Math.floor(Math.random() * (maxi - mini + 1)) + mini;
				setcouleur(i,j);
					
			}
				
		}	
	}
	
}
	
function start()
{
	if(document.getElementById('menu2').style.display!="none")
	{
		document.getElementById('menu2').style.display="none";
		document.getElementById('menu3').style.display="none";
		document.getElementById('menu6').style.display="block";
		document.getElementById('menu7').style.display="block";
		document.getElementById('bouton_change').value="reset";
		
		var tableauhtml="<table>";
		
		table=new Array(nbl);

		for(var i=0;i<nbl;i++)
		{
			tableauhtml+="<tr>";
			table[i]=new Array(nbc);

			for(var j=0;j<nbc;j++)
			{
				table[i][j]=parserInt(document.getElementById('td'+i+'_'+j).textContent);
				
				var stylecss=' style="background-color:' + getcouleur(table[i][j]) + ';';
				if (table[i][j]>3)
				{
					stylecss+='color:white;';
					elements_effondrables.push([i,j]);
					
				}
					
				stylecss+='" ';
				
				
				tableauhtml+='<td value="(' + i + ', '+j+')" id="td'+i+'_'+j+'" '+ stylecss +'  onmouseover="infocase('+i+', '+j+')" onclick="actioncase('+i+', '+j+')">';
				tableauhtml+=(document.getElementById('td'+i+'_'+j).textContent == "0")?"":document.getElementById('td'+i+'_'+j).textContent;
				tableauhtml+= '</td>';
				
			}

			tableauhtml+="</tr>";

		}

		tableauhtml+="</table>";
		
		document.getElementById('content').innerHTML=tableauhtml;
		document.getElementById('date').textContent=logical_date;
		
		style_cases();
		enveloppeconvexe();
		
		
		
	}
	
	else
	{
		document.getElementById('bouton_change').value="Commencer la simulation";
		stop_timer();
		init();	
	}
}

function enveloppeconvexe(){
	var i_0 = nbl;
	var i_1 = 0;
	var j_0 = nbc;
	var j_1 = 0;
	var valeur = 0;
	var nb0 = 0;
	var nb1 = 0;
	var nb2 = 0;
	var nb3 = 0;
	var nb4plus = 0;
	var masse=0;
	
	for(var i=0;i<nbl;i++){
		for(var j=0;j<nbc;j++){
			
				valeur=table[i][j];
				masse+=valeur;
			
			if(valeur != 0){
				if(valeur == 1){
					nb1++;
				}
				if(valeur == 2){
					nb2++;
				}
				if(valeur == 3){
					nb3++;
				}
				if(valeur>=4){
					nb4plus++;	
				}

				if(i<i_0){
					i_0=i;
				}
				if(i>i_1){
					i_1=i;
				}
				if(j<j_0){
					j_0=j;
				}
				if(j>j_1){
					j_1=j;
				}
			}
			else
			{
				nb0++;
			}
		}
	}
	var x = i_1-i_0+1;
	var y = j_1-j_0+1;
	document.getElementById('largeurenv').textContent=x.toString();
	document.getElementById('longueurenv').textContent=y.toString();
	document.getElementById('nb0').textContent=nb0.toString();
	document.getElementById('nb1').textContent=nb1.toString();
	document.getElementById('nb2').textContent=nb2.toString();
	document.getElementById('nb3').textContent=nb3.toString();
	document.getElementById('nb4').textContent=nb4plus.toString();
	document.getElementById('masse').textContent=masse.toString();
	
	nb_grains_case=[nb0,nb1,nb2,nb3,nb4plus];
	

}

function jouerauhasard(option) // option=normal, on ne joue plus au hasard en fait !
{
	var	lookforfour = elements_effondrables.length;
	var	compteur=0;
	var valeur=0;
	var intermediaire=[];
	
		
	if(lookforfour==0)
	{
		stop_timer();
		enveloppeconvexe();
		return 0;
	}
		
		if(option=="normal")
		{
			var case_choisie=lookforfour-1;
			intermediaire=elements_effondrables[case_choisie];
		}
		
		else
		{
			var case_choisie = Math.floor((Math.random() * lookforfour) + 0);
		
			intermediaire=elements_effondrables[case_choisie];
			elements_effondrables[case_choisie]=elements_effondrables[lookforfour-1];
			elements_effondrables[lookforfour-1]=intermediaire;
		}
			
			var i=intermediaire[0];
			var j=intermediaire[1];
		
		
		if(option=="normal")
		{
			ecroule(i,j,option);
		}
						
		else
		{
			ecroule(i,j);
		}
						
		return 1;
	
}

function finir()
{
	stop_timer();
	
	while(jouerauhasard("normal") != 0);
	
	for(var i=0;i<nbl;i++)
	{
		
		for(var j=0;j<nbc;j++)
		{
			var valeur= table[i][j];
			document.getElementById('td'+i+'_'+j).textContent = valeur>0?valeur:"";
			setcouleur(i,j);
		}
	}
	
	enveloppeconvexe();
	document.getElementById('date').textContent=logical_date;

}

function lancertimer()
{
	if(timer==0)
	{
		var timer_intervalle = parserInt(document.getElementById('timer_intervalle').value);
		timer=setInterval(function () {jouerauhasard();}, timer_intervalle);	
		document.getElementById('letimer').value="Arrêter le timer";
	}	
	else
	{
		stop_timer();
	}	
	
}

function stop_timer()
{
	if(timer!=0)
	{
		window.clearInterval(timer);
		timer=0;
		document.getElementById('letimer').value="Commencer le timer";	
	}
}